#pragma once

#include <godot_cpp/classes/ref.hpp>

#include "pathfinding_mesh.hpp"

using namespace godot;

struct PathfindingRegion
{
	StringName id = "region";
	Transform3D transform;
	Ref<PathfindingMesh> mesh;
};
