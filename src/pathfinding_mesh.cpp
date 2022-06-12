#include "pathfinding_mesh.hpp"

#include <godot_cpp/core/class_db.hpp>

void PathfindingMesh::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("set_cell_size", "cell_size"), &PathfindingMesh::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &PathfindingMesh::get_cell_size);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "cell_size"), "set_cell_size", "get_cell_size");

	ClassDB::bind_method(D_METHOD("set_elevation", "elevation"), &PathfindingMesh::set_elevation);
	ClassDB::bind_method(D_METHOD("get_elevation"), &PathfindingMesh::get_elevation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "elevation"), "set_elevation", "get_elevation");

	ClassDB::bind_method(D_METHOD("set_max_climb", "max_climb"), &PathfindingMesh::set_max_climb);
	ClassDB::bind_method(D_METHOD("get_max_climb"), &PathfindingMesh::get_max_climb);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_climb"), "set_max_climb", "get_max_climb");

	ClassDB::bind_method(D_METHOD("set_cells_bundled", "cells_bundled"), &PathfindingMesh::set_cells_bundled);
	ClassDB::bind_method(D_METHOD("get_cells_bundled"), &PathfindingMesh::get_cells_bundled);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cells_bundled"), "set_cells_bundled", "get_cells_bundled");
}

void PathfindingMesh::set_cell_size(const real_t &p_cell_size)
{
	cell_size = p_cell_size;
}

real_t PathfindingMesh::get_cell_size() const
{
	return cell_size;
}

void PathfindingMesh::set_elevation(const real_t &p_elevation)
{
	elevation = p_elevation;
}

real_t PathfindingMesh::get_elevation() const
{
	return elevation;
}

void PathfindingMesh::set_max_climb(const real_t &p_max_climb)
{
	max_climb = p_max_climb;
}

real_t PathfindingMesh::get_max_climb() const
{
	return max_climb;
}

void PathfindingMesh::set_cells(const std::vector<PathfindingCell> p_cells)
{
	std::map<Vector3i, PathfindingIndex> new_voxels;
	for (uint64_t i = 0; i < p_cells.size(); ++i)
	{
		const Vector3i voxel = p_cells[i].voxel;
		new_voxels[voxel] = i;
	}

	cells = p_cells;
	voxels = new_voxels;
}

std::vector<PathfindingCell> &PathfindingMesh::get_cells()
{
	return cells;
}

void PathfindingMesh::set_cells_bundled(const Array &p_cells_bundled)
{
	std::vector<PathfindingCell> new_cells;

	int i = 0;
	while (i < p_cells_bundled.size())
	{
		const Vector3i voxel = p_cells_bundled[i++];
		const Vector3 position = p_cells_bundled[i++];
		int connection_count = p_cells_bundled[i++];
		std::map<PathfindingIndex, PathfindingConnection> connections;
		for (int c = 0; c < connection_count; ++c)
		{
			const PathfindingIndex target = p_cells_bundled[i++];
			const real_t weight = p_cells_bundled[i++];
			const int obstacle_count = p_cells_bundled[i++];
			std::vector<PathfindingIndex> obstacles;
			for (int o = 0; o < obstacle_count; ++o)
			{
				obstacles.push_back(p_cells_bundled[i++]);
			}
			connections[target] = PathfindingConnection{
				.weight = weight,
				.obstacles = obstacles,
			};
		}
		new_cells.push_back(PathfindingCell{
			.voxel = voxel,
			.position = position,
			.connections = connections,
		});
	}
	ERR_FAIL_COND(i != p_cells_bundled.size());

	set_cells(new_cells);
}

Array PathfindingMesh::get_cells_bundled() const
{
	Array cells_bundled;
	for (const PathfindingCell &cell : cells)
	{
		cells_bundled.push_back(cell.voxel);
		cells_bundled.push_back(cell.position);
		cells_bundled.push_back(cell.connections.size());
		for (const std::pair<PathfindingIndex, PathfindingConnection> connection : cell.connections)
		{
			cells_bundled.push_back(connection.first);
			cells_bundled.push_back(connection.second.weight);
			cells_bundled.push_back(connection.second.obstacles.size());
			for (const PathfindingIndex &obstacle : connection.second.obstacles)
			{
				cells_bundled.push_back(obstacle);
			}
		}
	}
	return cells_bundled;
}

size_t PathfindingMesh::get_cell_count() const
{
	return cells.size();
}

PathfindingCell &PathfindingMesh::get_cell(const PathfindingIndex &p_index)
{
	return cells[p_index];
}

bool PathfindingMesh::find_cell(const Vector3i &p_voxel, PathfindingIndex &r_index) const
{
	std::map<Vector3i, PathfindingIndex>::const_iterator it = voxels.find(p_voxel);
	if (likely(it != voxels.end()))
	{
		r_index = it->second;
		return true;
	}
	return false;
}
