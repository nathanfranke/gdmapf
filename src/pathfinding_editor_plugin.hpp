#pragma once

#ifdef DEBUG_ENABLED

/*#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/control.hpp>

#include "pathfinding_region_3d.hpp"

class PathfindingEditor : public Control
{
	friend class PathfindingEditorPlugin;

	GDCLASS(PathfindingEditor, Control);

	Button *button_bake;

	PathfindingRegion3D *node;

	void _bake_pressed();

protected:
	void _node_removed(Node *p_node);
	static void _bind_methods();

public:
	void edit(PathfindingRegion3D *p_region);
	PathfindingEditor();
	~PathfindingEditor();
};

class PathfindingEditorPlugin : public EditorPlugin
{
	GDCLASS(PathfindingEditorPlugin, EditorPlugin);

	PathfindingEditor *Pathfinding_editor;
	EditorNode *editor;

public:
	virtual String get_name() const override { return "Pathfinding"; }
	bool has_main_screen() const override { return false; }
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;
	virtual void make_visible(bool p_visible) override;

	PathfindingEditorPlugin();
	~PathfindingEditorPlugin();
};*/

#endif
