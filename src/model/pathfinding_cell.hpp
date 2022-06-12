#pragma once

#include <map>
#include <vector>
#include <godot_cpp/core/math.hpp>

#include "pathfinding_index.hpp"

using namespace godot;

struct PathfindingConnection
{
	real_t weight;
	std::vector<PathfindingIndex> obstacles;
};

struct PathfindingCell
{
	Vector3i voxel;
	Vector3 position;
	std::map<PathfindingIndex, PathfindingConnection> connections;

#ifdef DEBUG_ENABLED
	real_t debug_density;
	real_t debug_potential;
#endif
};
