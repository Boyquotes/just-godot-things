/*******************************************************************************
 ACTOR: PLAYER CLASS

Node Tree Setup:
- KinematicBody "name"
	- CollisionShape "c"
	- CollisionShape "c_lo" (crouching collision)
	- Camera "cam"
		- ViewportContainer "vc_wep"
			- Viewport "vp_wep"
				- Camera "wep_view"
	- Spatial "modelname"	(GLTF scene)
		- Spatial	(rig name)
			- Skeleton "Skeleton"
				-MeshInstance	(mesh name)
		- AnimationPlayer "AnimationPlayer"
	- Area "water_2"
		- CollisionShape "c"
	- Area "water_3"
		- CollisionShape "c"
	- OmniLight "pwr_light"
	- ColorRect "screen_shader"
	- Control "hud"
	- Control "menu"
	- Control "devcon"
	- AnimationPlayer "AnimationPlayer"
	- AudioStreamPlayer3D "sfx0"	(VOICE)
	- AudioStreamPlayer3D "sfx1"	(WEAPON)
	- AudioStreamPlayer3D "sfx2"	(ITEM)
	- AudioStreamPlayer3D "sfx3"	(BODY)
	- Timer "st" (state_timer)
*******************************************************************************/
#pragma once
#include "Actor.h"
#include <ShaderMaterial.hpp>
#include "Viewport.hpp"
#include "Node2D.hpp"
#include "Camera.hpp"
#include "OmniLight.hpp"
#include "SpotLight.hpp"
#include <Environment.hpp>
#include "AudioStreamPlayer.hpp"
#include "ControlsManager.h"
#include "SaveManager.h"
#include "Hud.h"

class Player :	public Actor
{
private:
	GODOT_SUBCLASS(Player, Actor);
	// Input
	ControlsManager* CTRL;
	WeaponManager* WPN;
	bool use_input = false;
	// Collision
	Area *water_2, *water_3;
	CollisionShape *col_stand, *col_crouch;
	// Navigation
	bool crouching = false;
	// Camera
	Camera* camera;
	float cam_x_rotation = 0.0f;
	// Inventory
	int items = 0;
	int weapons = 0;
	int ammo[WeaponManager::AMMO_TYPES];
	SpotLight* torch;
	OmniLight* wep_light;
	bool torch_on = false;
	float torch_power = 1.0f;
	// Weapon
	godot::Camera* wep_view;
	int wep_id = -1;
	bool wep_just_switched = false;
	float wep_cooldown = 0.0f, wep_alt_cooldown = 0.0f;
	// HUD
	Hud* hud;
	Ref<ShaderMaterial> screen_shader;
	OmniLight* powerup_light;
	// Sound
	Ref<AudioStreamSample> s_jump[3], s_softland, s_hardland, s_pain_lo[3], s_pain_mid[3], s_pain_hi[3], s_torch, s_torchdie, s_charge;

	// PRIVATE METHODS ===============================================================================

public:
	// PUBLIC METHODS ===============================================================================
	// Godot
	static void _register_methods();

	// Node Refs
	Camera* get_camera();

	// Control
	void player_input();
	void player_use();
	int get_attack_input();

	// Navigation
	void nav_rotate();
	void nav_set_direction(Basis basis_dir, Vector3 move_dir);
	//Vector3 nav_jump(Vector3 vel, float delta);
	void nav_land();
	void nav_stance(float delta);
	bool is_crouching();

	// Water Navigation
	void enter_water(Area* new_water);
	void exit_water(Area* new_water);
	void water_level_check();

	// Combat
	void wep_switch(int new_wep);
	void _wep_switch();
	void set_wep_cooldown(float cool);
	float get_wep_cooldown();
	void set_wep_alt_cooldown(float cool);
	float get_wep_alt_cooldown();
	bool have_wep(int id);
	bool add_wep(int id);
	void set_ammo(int ammo_type, int amount);
	int get_ammo(int ammo_type);
	bool add_ammo(int ammo_type, int amount);
	void use_ammo(int ammo_type, int amount);
	bool add_health(int amount);
	bool add_armor(int armor_class);
	void add_superdamage();
	void add_invincibility();
	void item_flash(float speed = 0.5f);
	void damage(int amount, Node* attack = nullptr, NodePath attacker = NodePath());

	// Inventory
	int get_items();
	bool has_item(int item_type);
	bool add_item(int item_type);
	bool use_item(int item_type);
	void save_items_to_start_status();

	// Audio
	void snd_charge();
	void snd_ductstep();

	// Scipting
	void exit_map();

	// Save Data
	Dictionary data_save();
	void data_load(Dictionary data);

	// State Management
	void state_enter();
	void state_idle(float delta);
	void state_physics(float delta);
	void state_exit();
	//void state_change(String new_state);

	// Base Processing
	void _init();
	void _ready();
};

