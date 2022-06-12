#include "pathfinding_mesh_generator.hpp"

#include <queue>
#include <vector>
#include <map>

#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_shape_query_parameters3d.hpp>
#include <godot_cpp/classes/physics_server3d.hpp>
#include <godot_cpp/classes/sphere_shape3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "pathfinding_region_3d.hpp"

PathfindingMeshGenerator *PathfindingMeshGenerator::singleton = nullptr;

void PathfindingMeshGenerator::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("bake", "region"), &PathfindingMeshGenerator::bake);
	ClassDB::bind_method(D_METHOD("clear", "region"), &PathfindingMeshGenerator::clear);
}

PathfindingMeshGenerator *PathfindingMeshGenerator::get_singleton()
{
	return singleton;
}

void PathfindingMeshGenerator::bake(const Variant p_region)
{
	PathfindingRegion3D *const region = Object::cast_to<PathfindingRegion3D>(p_region);
	ERR_FAIL_NULL(region);
	ERR_FAIL_COND(region->get_mesh().is_null());

	PhysicsDirectSpaceState3D *const state = region->get_world_3d()->get_direct_space_state();
	const Transform3D transform = region->get_global_transform();
	const Transform3D transform_inv = transform.affine_inverse();
	const real_t cell_size = region->get_mesh()->get_cell_size();
	const real_t elevation = region->get_mesh()->get_elevation();
	const real_t max_climb = region->get_mesh()->get_max_climb() * cell_size + 0.001;

	ERR_FAIL_COND_MSG(cell_size <= 0.0, "Cell size must be positive and non-zero.");

	std::vector<PathfindingCell> cells;
	std::map<Vector3i, PathfindingIndex> cell_voxels;

	const auto get_voxel = [&](const Vector3 &p_local_position) -> Vector3i
	{
		return (p_local_position / cell_size).floor();
	};

	const auto find_cell = [&](const Vector3 &p_local_position, PathfindingIndex &r_cell) -> bool
	{
		const Vector3i voxel = get_voxel(p_local_position);

		for (int dx = -1; dx <= 1; ++dx)
		{
			for (int dy = -1; dy <= 1; ++dy)
			{
				for (int dz = -1; dz <= 1; ++dz)
				{
					const std::map<Vector3i, PathfindingIndex>::iterator it = cell_voxels.find(voxel + Vector3i(dx, dy, dz));
					if (it != cell_voxels.end())
					{
						const PathfindingIndex &index = it->second;
						PathfindingCell &cell = cells[index];
						if (cell.position.distance_to(p_local_position) < cell_size / 2.0)
						{
							r_cell = index;
							return true;
						}
					}
				}
			}
		}
		return false;
	};

	const Vector3 relative[] = {
		{cell_size, 0.0, 0.0},
		{-cell_size, 0.0, 0.0},
		{0.0, 0.0, cell_size},
		{0.0, 0.0, -cell_size},
	};
	const Vector3 ground_offset(0.0, elevation, 0.0);
	const Vector3 climb_offset(0.0, max_climb, 0.0);

	std::queue<Vector3> active;
	active.push(Vector3(cell_size, 0.0, cell_size) / 2.0);
	while (!active.empty())
	{
		const Vector3 current = active.front();
		active.pop();

		// Ensure target does not have an obstacle.
		Ref<PhysicsShapeQueryParameters3D> shape_params;
		shape_params.instantiate();
		Ref<SphereShape3D> shape;
		shape.instantiate();
		shape->set_radius(0.001);
		shape_params->set_shape(shape);
		shape_params->set_transform(Transform3D().translated(transform.xform(current + climb_offset)));
		const Array shape_results = state->intersect_shape(shape_params, 1);
		if (!shape_results.is_empty())
		{
			continue; // Obstacle exists.
		}

		// Find ground position, if any.
		Ref<PhysicsRayQueryParameters3D> ray_params;
		ray_params.instantiate();
		ray_params->set_from(transform.xform(current + climb_offset));
		ray_params->set_to(transform.xform(current - climb_offset));
		const Dictionary ray_result = state->intersect_ray(ray_params);
		if (ray_result.is_empty())
		{
			continue; // No intersection.
		}

		// print_line(String() + "cast from " + transform.xform(current + climb_offset) + " to " + transform.xform(current - climb_offset));

		const Vector3 position = transform_inv.xform((Vector3)ray_result["position"]);

		// Ensure all four corners also intersect.
		const Vector3 corners[] = {
			{-cell_size * (real_t)0.375, 0.0, -cell_size * (real_t)0.375},
			{cell_size * (real_t)0.375, 0.0, -cell_size * (real_t)0.375},
			{-cell_size * (real_t)0.375, 0.0, cell_size * (real_t)0.375},
			{cell_size * (real_t)0.375, 0.0, cell_size * (real_t)0.375},
		};
		bool corners_valid = true;
		for (const Vector3 corner : corners)
		{
			Ref<PhysicsRayQueryParameters3D> ray_params;
			ray_params.instantiate();
			ray_params->set_from(transform.xform(position + corner + climb_offset));
			ray_params->set_to(transform.xform(position + corner - climb_offset));
			const Dictionary ray_result = state->intersect_ray(ray_params);
			if (ray_result.is_empty())
			{
				corners_valid = false;
				break; // No intersection.
			}
		}
		if (!corners_valid)
		{
			continue; // A corner did not intersect.
		}

		// Create a new cell, or skip if already exists.
		PathfindingIndex _cell;
		if (find_cell(position, _cell))
		{
			continue;
		}

		const PathfindingIndex index = cells.size();
		const Vector3i voxel = get_voxel(position);

		cells.push_back(PathfindingCell{
			.voxel = voxel,
			.position = position + ground_offset,
			.connections = {},
		});
		cell_voxels[voxel] = index;

		for (const Vector3 relative : relative)
		{
			active.push(position + relative);
		}
	}

	const std::pair<Vector3, std::vector<Vector3>> connect_relative[] = {
		{{cell_size, 0, 0}, {}},
		{{-cell_size, 0, 0}, {}},
		{{0, 0, cell_size}, {}},
		{{0, 0, -cell_size}, {}},
		{{cell_size, 0, cell_size}, {{cell_size, 0, 0}, {0, 0, cell_size}}},
		{{cell_size, 0, -cell_size}, {{cell_size, 0, 0}, {0, 0, -cell_size}}},
		{{-cell_size, 0, cell_size}, {{-cell_size, 0, 0}, {0, 0, cell_size}}},
		{{-cell_size, 0, -cell_size}, {{-cell_size, 0, 0}, {0, 0, -cell_size}}},
	};

	for (PathfindingCell &cell : cells)
	{
		for (const std::pair<Vector3, std::vector<Vector3>> &entry : connect_relative)
		{
			const Vector3 &relative = entry.first;
			const std::vector<Vector3> &relative_obstacles = entry.second;

			std::vector<PathfindingIndex> obstacles;

			// Ensure each extra position has a cell.
			for (const Vector3 rel : relative_obstacles)
			{
				const Vector3 pos = cell.position + rel;

				PathfindingIndex obstacle;
				if (!find_cell(pos, obstacle))
				{
					goto obstacles_invalid; // Obstacle cell does not exist (corner).
				}

				obstacles.push_back(obstacle);
			}
			goto obstacles_valid;
		obstacles_invalid:
			continue;
		obstacles_valid:

			PathfindingIndex next;
			if (!find_cell(cell.position + relative, next))
			{
				continue;
			}

			cell.connections[next] = PathfindingConnection{
				.weight = cell.position.distance_to(cells[next].position),
				.obstacles = obstacles,
			};
		}
	}

	region->get_mesh()->set_cells(cells);
}

void PathfindingMeshGenerator::clear(const Variant p_region)
{
	PathfindingRegion3D *const region = Object::cast_to<PathfindingRegion3D>(p_region);
	ERR_FAIL_NULL(region);

	region->get_mesh()->set_cells({});
}

PathfindingMeshGenerator::PathfindingMeshGenerator()
{
	singleton = this;
}

PathfindingMeshGenerator::~PathfindingMeshGenerator()
{
}
