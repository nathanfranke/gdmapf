#include "register_types.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "pathfinding_agent_3d.hpp"
#include "pathfinding_region_3d.hpp"
#include "pathfinding_mesh.hpp"
#include "pathfinding_server_3d.hpp"
#include "pathfinding_mesh_generator.hpp"

using namespace godot;

PathfindingServer3D *_pathfinding_server = nullptr;
PathfindingMeshGenerator *_pathfinding_mesh_generator = nullptr;

void gdextension_initialize(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		ClassDB::register_class<PathfindingAgent3D>();
		ClassDB::register_class<PathfindingRegion3D>();
		ClassDB::register_class<PathfindingMesh>();
		ClassDB::register_class<PathfindingServer3D>();
		ClassDB::register_class<PathfindingMeshGenerator>();

		_pathfinding_server = memnew(PathfindingServer3D);
		Engine::get_singleton()->register_singleton("PathfindingServer3D", PathfindingServer3D::get_singleton());

		_pathfinding_mesh_generator = memnew(PathfindingMeshGenerator);
		Engine::get_singleton()->register_singleton("PathfindingMeshGenerator", PathfindingMeshGenerator::get_singleton());
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR)
	{
		// TODO: Add editor plugin once <https://github.com/godotengine/godot-cpp/issues/640> resolved.
	}
}

void gdextension_terminate(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE)
	{
		Engine::get_singleton()->unregister_singleton("PathfindingServer3D");
		Engine::get_singleton()->unregister_singleton("PathfindingMeshGenerator");

		// TODO: Properly dispose singletons once <https://github.com/godotengine/godot/issues/62152> resolved.
		// memdelete(_pathfinding_server);
		// memdelete(_pathfinding_mesh_generator);
	}
}

extern "C"
{
	GDNativeBool GDN_EXPORT gdextension_init(const GDNativeInterface *p_interface, const GDNativeExtensionClassLibraryPtr p_library, GDNativeInitialization *r_initialization)
	{
		godot::GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

		init_obj.register_initializer(gdextension_initialize);
		init_obj.register_terminator(gdextension_terminate);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

		return init_obj.init();
	}
}
