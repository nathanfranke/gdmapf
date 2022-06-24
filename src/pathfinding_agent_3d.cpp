#include "pathfinding_agent_3d.hpp"

#include "pathfinding_server_3d.hpp"

void PathfindingAgent3D::_bind_methods()
{
	ClassDB::bind_method(D_METHOD("get_rid"), &PathfindingAgent3D::get_rid);

	ClassDB::bind_method(D_METHOD("set_region", "region"), &PathfindingAgent3D::set_region);
	ClassDB::bind_method(D_METHOD("get_region"), &PathfindingAgent3D::get_region);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "region"), "set_region", "get_region");

	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &PathfindingAgent3D::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &PathfindingAgent3D::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("navigate", "goal"), &PathfindingAgent3D::navigate);
	ClassDB::bind_method(D_METHOD("stop"), &PathfindingAgent3D::stop);

	ClassDB::bind_method(D_METHOD("is_navigating"), &PathfindingAgent3D::is_navigating);
	ClassDB::bind_method(D_METHOD("get_next_position"), &PathfindingAgent3D::get_next_position);
}

void PathfindingAgent3D::_agent_update_transform()
{
	PathfindingServer3D::get_singleton()->agent_set_position(rid, get_global_transform().origin);
}

void PathfindingAgent3D::_ready()
{
	// TODO: Use transform notification.
	// set_notify_transform(true);

	_agent_update_transform();
}

void PathfindingAgent3D::_process(double delta)
{
	// TODO: Use transform notification.
	_agent_update_transform();
}

RID PathfindingAgent3D::get_rid() const
{
	return rid;
}

void PathfindingAgent3D::set_region(const StringName &p_region)
{
	region = p_region;
	PathfindingServer3D::get_singleton()->agent_set_region(rid, region);
}

StringName PathfindingAgent3D::get_region() const
{
	return region;
}

void PathfindingAgent3D::set_radius(const real_t &p_radius)
{
	radius = p_radius;
	PathfindingServer3D::get_singleton()->agent_set_radius(rid, radius);
}

real_t PathfindingAgent3D::get_radius() const
{
	return radius;
}

void PathfindingAgent3D::navigate(const Vector3 &p_goal)
{
	PathfindingServer3D::get_singleton()->agent_navigate(rid, p_goal);
}

void PathfindingAgent3D::stop()
{
	PathfindingServer3D::get_singleton()->agent_stop(rid);
}

bool PathfindingAgent3D::is_navigating() const
{
	return PathfindingServer3D::get_singleton()->agent_is_navigating(rid);
}

Vector3 PathfindingAgent3D::get_next_position() const
{
	return PathfindingServer3D::get_singleton()->agent_get_next_position(rid);
}

PathfindingAgent3D::PathfindingAgent3D()
{
	rid = PathfindingServer3D::get_singleton()->agent_create();
}

PathfindingAgent3D::~PathfindingAgent3D()
{
	PathfindingServer3D::get_singleton()->agent_free(rid);
}
