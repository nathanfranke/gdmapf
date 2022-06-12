#ifdef DEBUG_ENABLED

/*#include "pathfinding_editor_plugin.hpp"

#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/box_container.hpp>

#include "pathfinding_mesh_generator.hpp"

void PathfindingEditor::_node_removed(Node *p_node)
{
	if (p_node == node)
	{
		node = nullptr;
		hide();
	}
}

void PathfindingEditor::_bake_pressed()
{
	ERR_FAIL_COND(!node);
	EditorProgress pr("pathfinding_mesh_generator", TTR("Generating Pathfinding Mesh..."), 1);
	pr.step("");
	PathfindingMeshGenerator::get_singleton()->bake(node);
}

void PathfindingEditor::edit(PathfindingRegion3D *p_region)
{
	if (p_region == nullptr || node == p_region)
	{
		return;
	}

	node = p_region;
}

void PathfindingEditor::_bind_methods()
{
}

PathfindingEditor::PathfindingEditor()
{
	button_bake = memnew(Button);
	button_bake->set_flat(true);
	button_bake->set_text(TTR("Bake Pathfinding Mesh"));
	button_bake->connect("pressed", callable_mp(this, &PathfindingEditor::_bake_pressed));
}

PathfindingEditor::~PathfindingEditor()
{
}

void PathfindingEditorPlugin::edit(Object *p_object)
{
	Pathfinding_editor->edit(Object::cast_to<PathfindingRegion3D>(p_object));
}

bool PathfindingEditorPlugin::handles(Object *p_object) const
{
	return p_object->is_class("PathfindingRegion3D");
}

void PathfindingEditorPlugin::make_visible(bool p_visible)
{
	if (p_visible)
	{
		Pathfinding_editor->button_bake->show();
	}
	else
	{
		Pathfinding_editor->button_bake->hide();
	}
}

PathfindingEditorPlugin::PathfindingEditorPlugin()
{
	Pathfinding_editor = memnew(PathfindingEditor);
	EditorNode::get_singleton()->get_main_control()->add_child(Pathfinding_editor);
	add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, Pathfinding_editor->button_bake);
	Pathfinding_editor->button_bake->hide();
}

PathfindingEditorPlugin::~PathfindingEditorPlugin()
{
}*/

#endif
