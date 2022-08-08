#pragma once

#include <map>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/mutex_lock.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/mutex.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/thread.hpp>
#include <godot_cpp/templates/rid_owner.hpp>

#include "model/pathfinding_agent.hpp"
#include "model/pathfinding_region.hpp"
#include "pathfinding_mesh.hpp"

using namespace godot;

class PathfindingServer3D : public Object
{
	GDCLASS(PathfindingServer3D, Object);

	static PathfindingServer3D *singleton;

public: // TODO
	Mutex mutex;

	RID_Owner<PathfindingRegion> regions;
	RID_Owner<PathfindingAgent> agents;

protected:
	static void _bind_methods();

public:
	static PathfindingServer3D *get_singleton();

	std::map<Vector3i, PathfindingIndex> generate_voxels(const std::vector<PathfindingCell> &p_cells) const;

	RID find_region(const StringName &p_id) const;

	RID region_create();
	void region_free(const RID &p_region);

	void region_set_id(const RID &p_region, const StringName &p_id);
	void region_set_mesh(const RID &p_region, const Ref<PathfindingMesh> &p_mesh);
	void region_set_transform(const RID &p_region, const Transform3D &p_transform);

	RID agent_create();
	void agent_free(const RID &p_agent);

	void agent_set_region(const RID &p_agent, const StringName &p_region);
	void agent_set_position(const RID &p_agent, const Vector3 &p_position);
	void agent_set_radius(const RID &p_agent, const real_t &p_radius);
	void agent_navigate(const RID &p_agent, const Vector3 &p_goal);
	void agent_stop(const RID &p_agent);
	bool agent_is_navigating(const RID &p_agent);

	Vector3 agent_get_next_position(const RID &p_agent);

	void process(const real_t &p_delta);

	PathfindingServer3D();
	virtual ~PathfindingServer3D();
};
