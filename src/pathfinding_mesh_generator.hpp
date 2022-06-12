#pragma once

#include <godot_cpp/core/object.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class PathfindingMeshGenerator : public Object
{
	GDCLASS(PathfindingMeshGenerator, Object);

	static PathfindingMeshGenerator *singleton;

protected:
	static void _bind_methods();

public:
	static PathfindingMeshGenerator *get_singleton();

	void bake(const Variant p_region);
	void clear(const Variant p_region);

	PathfindingMeshGenerator();
	~PathfindingMeshGenerator();
};
