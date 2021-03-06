#include "Player.h"
#include "City.h"
#include <Math.hpp>
#include <Range.hpp>
#include <GodotGlobal.hpp>
#include <Math.hpp>
#include <SceneTree.hpp>
#include <Viewport.hpp>
#include <WorldEnvironment.hpp>
#include <Environment.hpp>
#include <iostream>
# define M_PI 3.14159265358979323846  /* pi */

using namespace godot;


Vector3 center = Vector3(15.0 * citysize + 60, 0, 15.0 * citysize + 60);



Player::Player() {
	// INITIALIZE MOTION AND ROTATION VECTORS USED FOR PLAYER CONTROL
	motion = Vector3(0, 0, 0);
	rotation = Vector3(0, 0, 0);
	counter = 0;
}

Player::~Player() {
}

void Player::_register_methods() {
	register_method((char*)"_process", &Player::_process);
	register_method((char*)"_input", &Player::_input);
	register_method((char*)"_ready", &Player::_ready);
	register_property<Player, bool>("movable", &Player::movable, true);
}

void Player::_init() {
	movable = true;
}

void Player::_ready()
{
	Input* i = Input::get_singleton();
	i->set_mouse_mode(i->MOUSE_MODE_VISIBLE);
	WorldEnvironment* worldEnv = (WorldEnvironment*)(this->get_tree()->get_root()->get_node("Main")->get_node("3Dworld")->get_node("WorldEnvironment"));
	worldEnv->get_environment()->set_dof_blur_far_enabled(true);
	worldEnv->get_environment()->set_dof_blur_near_enabled(true);

	this->set("translation", StartPosition + center);
	this->set("rotation_degrees", StartRotation);

	this->update_camera_angle();

}

void Player::update_camera_angle()
{
	double h = ((Vector3)(this->get("translation"))).y;

	CameraAngleDeg = ((MaxCameraAngle - MinCameraAngle) / (MaxHeight - MinHeight)) * (h - MinHeight) + MinCameraAngle;

	this->get_node("ClippedCamera")->set("rotation_degrees", Vector3(-CameraAngleDeg, 0, 0));
}

void Player::_process(float delta)
{
	if (movable) { UpdateMotionFromInput(delta); }

	Vector3 position = this->get("translation");
	if (position.y > MaxHeight) {
		this->set("translation", Vector3(position.x, MaxHeight, position.z));
	}
	else if (position.y < MinHeight) {
		this->set("translation", Vector3(position.x, MinHeight, position.z));
	}

	WorldEnvironment* worldEnv = (WorldEnvironment*)(this->get_tree()->get_root()->get_node("Main")->get_node("3Dworld")->get_node("WorldEnvironment"));
	worldEnv->get_environment()->set_dof_blur_far_distance(2 * (this->get_global_transform().get_origin().y));
	worldEnv->get_environment()->set_dof_blur_far_amount(0.1 * pow(((MaxHeight - this->get_global_transform().get_origin().y) / (MaxHeight - MinHeight)), 0.2));

	this->get_child(0)->set("far", pow(get_global_transform().get_origin().y / MaxHeight, 0.4) * 600);
	if (get_global_transform().get_origin().y < 150) {
		worldEnv->get_environment()->set("glow_intensity", 5 * pow(((150.0 - this->get_global_transform().get_origin().y) / (MaxHeight - 150)), 1));
		worldEnv->get_environment()->set("glow_enabled", true);
	}
	else {
		worldEnv->get_environment()->set("glow_enabled", false);
	}
	this->translate(motion);
	this->set_rotation_degrees(rotation);
	this->update_camera_angle();

}

void Player::_physics_process(float delta)
{
	
}

void Player::_input(InputEvent* e)
{
	motion = Vector3(0, 0, 0);

	// MOUSE MOTION EVENTS

	if (e->get_class() == "InputEventMouseMotion")
	{
		// Rotation and vertical motion using relative mouse coordinates
		if (movable) { UpdateRotationFromInput((InputEventMouseMotion*)e); }
	}

	Input* i = Input::get_singleton();

	if (e->is_action_pressed("ui_turn"))
	{
		// HIDE NOTIFICATION BOX
		this->get_tree()->get_root()->get_node("Main/2Dworld/InvalidInputNotification")->set("visible", false); 

		mouse_p = this->get_viewport()->get_mouse_position();
		i->set_mouse_mode(i->MOUSE_MODE_CAPTURED);
	}

	if (e->is_action_released("ui_turn"))
	{
		i->set_mouse_mode(i->MOUSE_MODE_VISIBLE);
		this->get_viewport()->warp_mouse(mouse_p);
	}

	this->translate(motion);

	if (e->is_action_pressed("ui_cancel"))
	{
		// EXIT GAME
		get_tree()->quit();
	}

}

void Player::UpdateMotionFromInput(float delta)
{
	// RESET MOTION VECTOR TO ZERO
	motion = Vector3(0, 0, 0);

	// INPUT USED FOR KEY CONTROLS
	Input* i = Input::get_singleton();

	Vector3 pos = this->get("translation");
	SPEED_T = 2 * pos.y * delta;// *(citysize / 2 - pow((center.x - pos.x) * (center.x - pos.x) + (center.z - pos.z) * (center.z - pos.z), 0.5));

	// VERTICAL MOTION

	if (i->is_action_pressed("ui_vup")) {
		if (this->get_global_transform().get_origin().y <= MaxHeight - 10) { motion.y += SPEED_T; }
	}
	else if (i->is_action_pressed("ui_vdown")) {
		if (this->get_global_transform().get_origin().y >= MinHeight + 10) { motion.y -= SPEED_T; }
	}

	// PLANAR MOTION

	// 4-key combinations
	if ((i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_down")) && (i->is_action_pressed("ui_right")) && (i->is_action_pressed("ui_left"))) {}
	// 3-key combinations
	else if ((i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_right")) && (i->is_action_pressed("ui_left"))) { motion.z -= SPEED_T; }
	else if ((i->is_action_pressed("ui_down")) && (i->is_action_pressed("ui_right")) && (i->is_action_pressed("ui_left"))) { motion.z += SPEED_T; }
	else if ((i->is_action_pressed("ui_right")) && (i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_down"))) { motion.x += SPEED_T; }
	else if ((i->is_action_pressed("ui_left")) && (i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_down"))) { motion.x -= SPEED_T; }
	// 2-key combinations
	else if ((i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_down"))) {}
	else if ((i->is_action_pressed("ui_left")) && (i->is_action_pressed("ui_right"))) {}
	else if ((i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_right"))) {
		motion.z -= SPEED_T / (sqrt(2));
		motion.x += SPEED_T / (sqrt(2));
	}
	else if ((i->is_action_pressed("ui_down")) && (i->is_action_pressed("ui_right"))) {
		motion.z += SPEED_T / (sqrt(2));
		motion.x += SPEED_T / (sqrt(2));
	}
	else if ((i->is_action_pressed("ui_up")) && (i->is_action_pressed("ui_left"))) {
		motion.z -= SPEED_T / (sqrt(2));
		motion.x -= SPEED_T / (sqrt(2));
	}
	else if ((i->is_action_pressed("ui_down")) && (i->is_action_pressed("ui_left"))) {
		motion.z += SPEED_T / (sqrt(2));
		motion.x -= SPEED_T / (sqrt(2));
	}
	// 1-key combinations
	else if (i->is_action_pressed("ui_up")) { motion.z -= SPEED_T; }
	else if (i->is_action_pressed("ui_down")) { motion.z += SPEED_T; }
	else if (i->is_action_pressed("ui_right")) { motion.x += SPEED_T; }
	else if (i->is_action_pressed("ui_left")) { motion.x -= SPEED_T; }

}

void Player::UpdateRotationFromInput(InputEventMouseMotion* e) {
	Vector2 rot = e->get_relative();						// RELATIVE ROTATION VECTOR OBTAINED FROM MOUSE MOTION
	Input* i = Input::get_singleton();						// INPUT USED FOR MOUSE MODE

	if (i->get_mouse_mode() == i->MOUSE_MODE_CAPTURED)		// CHANGE PLAYER ROTATION ONLY IF MOUSE IS CAPTURED
	{
		// LOOK LEFT/RIGHT
		rotation.y -= rot.x * (SPEED_R / 512);				//Must depends on the screen size to avoid slower rotation on high def screens

		// ZOOM-IN MOTION 
		if (rot.y <= 0) {
			if (this->get_global_transform().get_origin().y <= MaxHeight - 10)	// Set maximum height
			{
				motion.z -= (rot.y * cos((MaxCameraAngle + MinCameraAngle) * (M_PI) / 180)) / (VSPEED_INVERSE / pow(this->get_global_transform().get_origin().y - MinHeight / 2, VSPEED_POWER));
				motion.y -= (rot.y * sin((MaxCameraAngle + MinCameraAngle) * (M_PI) / 180)) / (VSPEED_INVERSE / pow(this->get_global_transform().get_origin().y - MinHeight / 2, VSPEED_POWER));
			}
		}
		else {
			// Set minimum height
			if (this->get_global_transform().get_origin().y >= MinHeight + 10)
			{
				motion.z -= (rot.y * cos((MaxCameraAngle + MinCameraAngle) * (M_PI) / 180)) / (VSPEED_INVERSE / pow(this->get_global_transform().get_origin().y - MinHeight / 2, VSPEED_POWER));
				motion.y -= (rot.y * sin((MaxCameraAngle + MinCameraAngle) * (M_PI) / 180)) / (VSPEED_INVERSE / pow(this->get_global_transform().get_origin().y - MinHeight / 2, VSPEED_POWER));
			}
		}
	}

}

void Player::ChangeMouseMode() {
	Input* i = Input::get_singleton();

	// CHANGE MOUSE MODE
	if (i->get_mouse_mode() == i->MOUSE_MODE_CAPTURED)
	{
		i->set_mouse_mode(i->MOUSE_MODE_VISIBLE);
	}
	else {
		i->set_mouse_mode(i->MOUSE_MODE_CAPTURED);
	}
}