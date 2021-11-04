extends Node


# Declare member variables here. Examples:
# var a = 2
# var b = "text"
var ref : Recorder = null

# Called when the node enters the scene tree for the first time.
func _ready():
	ref = $ReferenceRect
	pass # Replace with function body.

var do_once : bool = true
var do_once2 : bool = true
var timer : float = 0.0

func _process(delta : float) -> void:
	timer += delta
	if timer > 0.5 and do_once:
		do_once = false
		ref.record_duration(5)
	if timer > 15 and do_once2:
		do_once2 = false
		get_tree().quit()

# func _input(event):
# 	if event.is_action_pressed("record"):
# 		ref.record_duration(2)
# 	pass
