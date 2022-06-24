#include "pathfinding_region_3d.hpp"

#include <godot_cpp/classes/engine.hpp>

#include "pathfinding_server_3d.hpp"

void PathfindingRegion3D::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_rid"), &PathfindingRegion3D::get_rid);

	ClassDB::bind_method(D_METHOD("set_action_generate", "value"), &PathfindingRegion3D::set_action_generate);
	ClassDB::bind_method(D_METHOD("get_action_generate"), &PathfindingRegion3D::get_action_generate);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "action_generate"), "set_action_generate", "get_action_generate");

	ClassDB::bind_method(D_METHOD("set_id", "id"), &PathfindingRegion3D::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &PathfindingRegion3D::get_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "id"), "set_id", "get_id");

	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &PathfindingRegion3D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_mesh"), &PathfindingRegion3D::get_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "PathfindingMesh"), "set_mesh", "get_mesh");
}

#ifdef DEBUG_ENABLED
void PathfindingRegion3D::_debug_init()
{
	debug = memnew(MeshInstance3D);

	debug_mesh.instantiate();
	debug->set_mesh(debug_mesh);

	add_child(debug);

	for (float h = 0.0; h < 1.0; h += 0.01)
	{
		Ref<StandardMaterial3D> mat;
		mat.instantiate();
		mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
		mat->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
		mat->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
		Color c;
		c.set_hsv(h, 1.0, 1.0);
		mat->set_albedo(c);
		debug_cell_materials.push_back(mat);
	}

	debug_region_material.instantiate();
	debug_region_material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	debug_region_material->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
	debug_region_material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	debug_region_material->set_albedo(Color(1.0, 0.0, 0.0));

	_debug_update();
}

void PathfindingRegion3D::_debug_update()
{
	if (mesh.is_null())
	{
		return;
	}

	if (!is_visible_in_tree())
	{
		return;
	}

	const auto wireframe_cube = [](const Vector3 &p_pos, const Vector3 &p_size, PackedVector3Array &r_lines) -> void
	{
		const Vector3 nnn = p_pos;
		const Vector3 xnn = p_pos + Vector3(p_size.x, 0.0, 0.0);
		const Vector3 nyn = p_pos + Vector3(0.0, p_size.y, 0.0);
		const Vector3 nnz = p_pos + Vector3(0.0, 0.0, p_size.z);
		const Vector3 xyn = p_pos + Vector3(p_size.x, p_size.y, 0.0);
		const Vector3 nyz = p_pos + Vector3(0.0, p_size.y, p_size.z);
		const Vector3 xnz = p_pos + Vector3(p_size.x, 0.0, p_size.z);
		const Vector3 xyz = p_pos + p_size;

		// Front
		r_lines.push_back(nnn);
		r_lines.push_back(xnn);
		r_lines.push_back(xnn);
		r_lines.push_back(xyn);
		r_lines.push_back(nnn);
		r_lines.push_back(nyn);
		r_lines.push_back(nyn);
		r_lines.push_back(xyn);

		// Back
		r_lines.push_back(nnz);
		r_lines.push_back(xnz);
		r_lines.push_back(xnz);
		r_lines.push_back(xyz);
		r_lines.push_back(nnz);
		r_lines.push_back(nyz);
		r_lines.push_back(nyz);
		r_lines.push_back(xyz);

		// Connection
		r_lines.push_back(nnn);
		r_lines.push_back(nnz);
		r_lines.push_back(xnn);
		r_lines.push_back(xnz);
		r_lines.push_back(xyn);
		r_lines.push_back(xyz);
		r_lines.push_back(nyn);
		r_lines.push_back(nyz);
	};

	debug_mesh->clear_surfaces();

	Array a;
	a.resize(Mesh::ARRAY_MAX);

	int surface_count = 0;

	std::map<int, PackedVector3Array> lines;

	// Cells
	for (PathfindingIndex index = 0; index < mesh->get_cell_count(); ++index)
	{
		const PathfindingCell &cell = mesh->get_cell(index);
		PackedVector3Array &verts = lines[(int)(Math::fposmod(cell.debug_potential * 0.1, 1.0) * debug_cell_materials.size())];

		if (cell.debug_density >= 1000.0)
		{
			continue;
		}

		wireframe_cube(cell.position - Vector3(0.3, 0.3, 0.3) * mesh->get_cell_size(), Vector3(0.6, 0.6, 0.6) * mesh->get_cell_size(), verts);

		real_t best_pot = cell.debug_potential - cell.debug_density;
		PathfindingCell best_cell = cell;

		for (const std::pair<const PathfindingIndex, PathfindingConnection> &connection : cell.connections)
		{
			/*verts.push_back(cell.position + Vector3(0, 0.1, 0));
			verts.push_back(cell.position.move_toward(mesh->get_cell(connection.first).position, 0.3) + Vector3(0, 0.1, 0));*/

			const PathfindingIndex &next_cell = connection.first;
			const PathfindingCell &next = mesh->get_cell(next_cell);
			const real_t pot = next.debug_potential;

			if (next.debug_density > 0.0)
			{
				goto invalid;
			}
			for (const PathfindingIndex obstacle : connection.second.obstacles)
			{
				if (mesh->get_cell(obstacle).debug_density > 0.0)
				{
					goto invalid;
				}
			}
			goto valid;
		invalid:
			continue;
		valid:;

			if (pot < best_pot)
			{
				best_pot = pot;
				best_cell = next;
			}
		}

		verts.push_back(cell.position + Vector3(0, 0.1, 0));
		verts.push_back(cell.position.move_toward(best_cell.position, 0.3) + Vector3(0, 0.1, 0));
	}

	for (int i = 0; i < debug_cell_materials.size(); ++i)
	{
		const PackedVector3Array &verts = lines[i];
		if (!verts.is_empty())
		{
			a[Mesh::ARRAY_VERTEX] = verts;
			debug_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, a);
			debug_mesh->surface_set_material(surface_count++, debug_cell_materials[i]);
		}
	}
}
#endif

void PathfindingRegion3D::_region_update_transform()
{
	PathfindingServer3D::get_singleton()->region_set_transform(rid, get_global_transform());
}

void PathfindingRegion3D::_ready()
{
	// TODO: Use transform notification.
	// set_notify_transform(true);

	_region_update_transform();

#ifdef DEBUG_ENABLED
	if (Engine::get_singleton()->is_editor_hint())
	{
		_debug_init();
	}
#endif
}

void PathfindingRegion3D::_process(double delta)
{
	// TODO: Use transform notification.
	_region_update_transform();

#ifdef DEBUG_ENABLED
	if (Engine::get_singleton()->is_editor_hint())
	{
		_debug_update();
	}
#endif
}

RID PathfindingRegion3D::get_rid() const
{
	return rid;
}

void PathfindingRegion3D::set_id(const StringName &p_id)
{
	id = p_id;
	PathfindingServer3D::get_singleton()->region_set_id(rid, p_id);
}

StringName PathfindingRegion3D::get_id() const
{
	return id;
}

void PathfindingRegion3D::set_mesh(const Ref<PathfindingMesh> &p_mesh)
{
	mesh = p_mesh;
	PathfindingServer3D::get_singleton()->region_set_mesh(rid, p_mesh);
}

Ref<PathfindingMesh> PathfindingRegion3D::get_mesh() const
{
	return mesh;
}

PathfindingRegion3D::PathfindingRegion3D()
{
	rid = PathfindingServer3D::get_singleton()->region_create();
}

PathfindingRegion3D::~PathfindingRegion3D()
{
	PathfindingServer3D::get_singleton()->region_free(rid);
}
