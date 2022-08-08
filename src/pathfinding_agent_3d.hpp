#pragma once

#include <vector>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include "pathfinding_mesh.hpp"

class PathfindingAgent3D : public Node3D
{
	GDCLASS(PathfindingAgent3D, Node3D);

	RID rid;

	StringName region = "region";
	real_t radius = 0.5;

protected:
	static void _bind_methods();

	void _agent_update_transform();

public:
	void _ready() override;
	void _process(double delta) override;

	RID get_rid() const;

	void set_region(const StringName &p_region);
	StringName get_region() const;

	void set_radius(const real_t &p_radius);
	real_t get_radius() const;

	void navigate(const Vector3 &p_goal);
	void stop();
	bool is_navigating() const;
	Vector3 get_next_position() const;

	PathfindingAgent3D();
	virtual ~PathfindingAgent3D();
};
