#pragma once

#include <vector>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "pathfinding_mesh.hpp"
#include "pathfinding_mesh_generator.hpp"

using namespace godot;

class PathfindingRegion3D : public Node3D
{
	GDCLASS(PathfindingRegion3D, Node3D);

	RID rid;

	bool action_generate = false;
	StringName id = "region";
	Ref<PathfindingMesh> mesh;

#ifdef DEBUG_ENABLED
	MeshInstance3D *debug = nullptr;
	Ref<ArrayMesh> debug_mesh;
	Vector<Ref<StandardMaterial3D>> debug_cell_materials;
	Ref<StandardMaterial3D> debug_region_material;
#endif

protected:
	static void _bind_methods();

#ifdef DEBUG_ENABLED
	void _debug_init();
	void _debug_update();
#endif

	void _region_update_transform();

public:
	void _ready() override;
	void _process(double delta) override;

	// TODO: Replace with editor plugin <https://github.com/godotengine/godot-cpp/issues/640>.
	void set_action_generate(const bool &p_value)
	{
		PathfindingMeshGenerator::get_singleton()->bake(this);
	}
	bool get_action_generate() const
	{
		return false;
	}

	RID get_rid() const;

	void set_id(const StringName &p_id);
	StringName get_id() const;

	void set_mesh(const Ref<PathfindingMesh> &p_mesh);
	Ref<PathfindingMesh> get_mesh() const;

	PathfindingRegion3D();
	virtual ~PathfindingRegion3D();
};
