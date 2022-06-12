extends Node3D

@onready var agent: PathfindingAgent3D = $PathfindingAgent3D
@onready var material: StandardMaterial3D = $MeshInstance3D.material_override

@export var big := false

func _process(_delta: float) -> void:
	if agent.is_navigating():
		material.albedo_color = Color.YELLOW
	else:
		material.albedo_color = Color.GREEN

func _physics_process(delta: float) -> void:
	#print(agent.get_next_position())
	global_transform.origin = global_transform.origin.move_toward(agent.get_next_position(), delta * 3)

## Tells the agent to move to [code]target[/code].
func navigate(target: Vector3) -> void:
	agent.navigate(target)
