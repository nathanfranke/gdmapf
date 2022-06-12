#pragma once

#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

struct PathfindingAgent
{
	// Region this agent is navigating on.
	StringName region = "region";
	// Current position of the agent.
	Vector3 position;
	// Radius of the agent.
	real_t radius = 0.5;
	// Whether the agent is navigating.
	bool is_navigating = false;
	// End goal of the agent.
	Vector3 goal;
	// Next position to approach.
	Vector3 next_position;
};
