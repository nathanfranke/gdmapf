extends Node3D

func _physics_process(delta: float) -> void:
	PathfindingServer3D.process(delta)

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				var camera := get_viewport().get_camera_3d()
				var origin := camera.project_ray_origin(event.position)
				var normal := camera.project_ray_normal(event.position)
				
				var params := PhysicsRayQueryParameters3D.new()
				params.from = origin
				params.to = origin + normal * 1000.0
				
				var space := get_world_3d().direct_space_state
				var result := space.intersect_ray(params)
				
				if result.has("position"):
					var pos: Vector3 = result.position
					print("Navigating all to ", pos)
					for agent in get_tree().get_nodes_in_group("agent"):
						if agent.big == event.shift_pressed:
							agent.navigate(pos)
