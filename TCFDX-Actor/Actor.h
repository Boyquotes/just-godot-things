/*******************************************************************************
ACTOR 
Base class for all actor types, mainly players and monsters. Some types of
objects may use this class, like grenades, for the purposes of taking advantage
of the gravity and damage mechanics.

Node Tree Setup:
- KinematicBody "name"
	- CollisionShape "c"
	- Spatial "modelname" (GLTF)
		- Spatial "rigname"
			- Skeleton "Skeleton"
				-MeshInstance "meshname"
		- AnimationPlayer "AnimationPlayer"
	- AnimationPlayer "AnimationPlayer"
	- AudioStreamPlayer3D "sfx0" (VOICE)
	- AudioStreamPlayer3D "sfx1" (WEAPON)
	- AudioStreamPlayer3D "sfx2" (ITEM)
	- AudioStreamPlayer3D "sfx3" (BODY)
*******************************************************************************/
#pragma once
#include "Common.h"
#include <functional>
#include <vector>
#include <map>
#include <algorithm>
#include "KinematicBody.hpp"
#include <KinematicCollision.hpp>
#include "Shape.hpp"
#include <World.hpp>
#include <PhysicsDirectSpaceState.hpp>
#include <PhysicsShapeQueryParameters.hpp>
#include "AnimationPlayer.hpp"
#include "Area.hpp"
#include "Timer.hpp"
#include "CollisionShape.hpp"
#include "SoundManager.h"
#include "GameManager.h"
#include "AiManager.h"
#include "Gib.h"
#include "PathDx.h"

class Actor : public KinematicBody
{
private:
	GODOT_CLASS(Actor, KinematicBody);
protected:
	// Autoload References
	GameManager* GAME; AiManager* AIM; SoundManager* SND;
	PhysicsDirectSpaceState* space_state;
public:
	// PROTECTED VARIABLES ==================================================
	String classname = "";
	Dictionary properties;
	int spawnflags = GameManager::FL_NOT_IN_DEATHMATCH + GameManager::FL_NOT_IN_TEAMDEATHMATCH;
	int actorflags = GameManager::FL_MONSTER;
	// Health Management
	int health = 100, health_max = 100, armor = 0, armor_max = 0;
	float armor_rating = 0.666f, health_rot_ct = 1.0f;
	// State Management
	enum STATES {
		// IMPORTANT! DO NOT CHANGE THE ORDER OF THESE ENUMS! ONLY INSERT NEW STATE ENUMS BEFORE ST_DEADSTART!
		// Some animations call state_change() during playback.
		ST_IDLE, ST_DEAD, ST_PAIN, ST_AMBUSH, ST_CHASE, ST_PATHING, ST_ATTACK, ST_LEAP, ST_LAND, ST_DASH,
		ST_GRAB, ST_PULL, ST_EAT, ST_WORSHIP, ST_SLEEP, ST_WAKE, ST_SCARED,
		ST_DEADSTART = 125, ST_GIBSTART, ST_TELESPAWN, ST_REMOVED, ST_START
	};
	int current_state = ST_START, previous_state = ST_START;
	bool think_check = false;
	float state_timer = 0.0f, next_think = 0.0f, queue_timer = 0.0f;
	String think = "";
	// Collision
	CollisionShape* col_node;
	Ref<Shape> col_shape;
	float col_radius = 0.5f, col_floor = 1.0f;
	Array col_ex_self;
	// Navigation
	Vector3 grav_dir = Vector3(0, -1, 0), grav_vector = Vector3::ZERO;
	Vector3 velocity = Vector3::ZERO;
	float teleport_delay = 0.0f;
	float walk_speed = 4.5f, run_speed = 10.0f, max_speed = 10.0f, stop_speed = 3.125f;
	float friction = 4.0f, friction_delay = 0.0f, acceleration = 10.0f, air_acceleration = 0.7f;
	float jump_strength = 8.4375f, max_fall_speed = 1.0f;
	bool jumping = false, flying = false, on_floor = true, check_bottom = true;
	Vector3 grav_accel = Vector3::ZERO;
	Vector3 nav_dir = Vector3::ZERO, nav_target_pos = Vector3::ZERO;
	Dictionary nav_floor;
	// Water Navigation
	float water_acceleration = 10.0f, water_friction = 4.0f, water_jump_delay = 0.0f;
	Area* water_vol;
	int water_type = 0, water_level = 0;
	// Targeting
	Array chase_trail;
	float hearing_range = 1024.0f;
	// Combat
	std::vector<String> pain_anims, death_anims;
	std::vector<Ref<PackedScene>> gib_res = {};
	float weight = 1.0f, damaged = 0.0f, shielding = 0.0f, superdamage = 0.0f, invincibility = 0.0f;
	int bleed_type = 0, pain_chance = -1, gib_threshold = -40;
	bool gibbed = false;
	NodePath grabbed_by = NodePath();
	// Monster Ai
	bool in_pvs = false;
	std::vector<String> enemy_groups;
	Spatial* enemies[100] = { nullptr };
	int enemy_search_index = 0;
	enum PATH { ONCE, LOOP, PINGPONG };
	std::vector<PathDx*> path_list = {};
	int next_check = 1, path_index = 0, path_loop_type = ONCE;
	NodePath enemy_path = NodePath("");
	Spatial* enemy = nullptr;
	Vector3 last_enemy_pos = Vector3::ZERO;
	bool mad = false, stationary = false, aim_queued = false;
	float hunt_time = 0.0f;
	String path_name = "";
	// Animation
	AnimationPlayer* anim_player;
	// Sound
	std::vector<Ref<AudioStreamSample>> s_mad = {}, s_pain = {}, s_die = {};
	AudioStreamPlayer3D* sfx[4] = { nullptr };
	// Scripting
	String trg_target, trg_targetfunc, trg_message;
	// Misc
	Ref<RandomNumberGenerator> rng;
	// Input
	Vector3 move_input = Vector3::ZERO;
	int attack_input = 0;

	// METHODS ============================================================
	static void _register_methods();
	// PROPERTIES ----------------------------------------
	void set_properties(Dictionary new_properties);	Dictionary get_properties();
	void set_classname(String n); String get_classname();
	void set_spawnflags(int x);	int get_spawnflags();
	void set_trg_target(String t); String get_trg_target();
	void set_trg_targetfunc(String f); String get_trg_targetfunc();
	void set_trg_message(String m); String get_trg_message();

	// COLLISION ------------------------------------
	// Raycasting, useful for everything
	Dictionary col_ray(Vector3 origin, Vector3 cast_to, int mask, Array exclude);
	Dictionary col_ray_body(Vector3 origin, Vector3 cast_to, int mask, Array exclude);
	Dictionary col_ray_area(Vector3 origin, Vector3 cast_to, int mask, Array exclude);
	// Shape checking, useful for telefrags among other things
	Ref<PhysicsShapeQueryParameters> col_make_shape_query(Transform shape_transform, float shape_margin, int mask, Array exclude);
	Array col_shape_check(Transform shape_transform, float shape_margin, int mask, Array exclude);
	Array col_shape_check_body(Transform shape_transform, float shape_margin, int mask, Array exclude);
	Array col_shape_check_area(Transform shape_transform, float shape_margin, int mask, Array exclude);
	Array col_cast_motion(float shape_margin, int mask, Array exclude, Vector3 motion);
	// Collision layer setting
	float get_col_floor();
	float get_col_radius();
	void col_set_solid();
	void col_set_dead();
	void set_noclip(bool is_noclip);
	bool get_noclip();

	// NAVIGATION -----------------------------------
	void nav_floor_update();
	Dictionary get_nav_floor();
	bool has_nav_floor();
	bool nav_grav_dir();
	void nav_xform(float delta = -1.0f);
	void nav_grav_accel(float delta);
	void nav_set_direction(Basis basis_dir, Vector3 move_dir);
	Vector3 nav_friction(Vector3 vel, float delta);
	Vector3 nav_accelerate(Vector3 vel, float delta);
	Vector3 nav_air_accelerate(Vector3 vel, float delta);
	Vector3 nav_jump(Vector3 vel, float delta);
	void nav_move(float delta);
	void nav_fly_move(float delta);
	bool nav_check_bottom(Vector3 offset = Vector3::ZERO);
	bool nav_check_move(Vector3 offset = Vector3::ZERO);
	Vector3 get_move_vec();
	void set_move_input(Vector3 new_move);
	Vector3 get_move_input();
	void set_jump(bool j = true);
	void teleport(Transform dest_xform);
	void grav_set(Transform xform);
	void grav_set_dir(Vector3 new_grav_dir);
	void set_grav_dir(Vector3 new_grav_dir);
	Vector3 get_grav_dir();
	void set_velocity(Vector3 v); Vector3 get_velocity();
	void set_velocity_local(Vector3 v);
	void face_pos(Vector3 tgt_pos, float turn_speed);
	void enter_water(Area* water);
	void exit_water(Area* water);
	void sv_speed(float new_spd = 10.0f);
	void sv_friction(float new_frc = 4.0f);
	void sv_jump(float new_jmp = 8.4375f);
	void sv_weight(float new_wgt = 1.0f);
	void set_flying(bool is_flying); bool get_flying();
	void set_friction_delay(float delay);
	
	// TARGETING ------------------------------------
	Vector3 get_pos_dir(Vector3 tgt_pos, Vector3 axis);
	float get_pos_dist(Vector3 tgt_pos, Vector3 axis);
	void turn_towards_pos(float delta, Vector3 tgt_pos, float turn_speed = 10.0f);
	bool line_of_sight(Vector3 tgt_pos, float fov = 0.3f);
	void chase_add_breadcrumb(float spacing = 1.0f);
	Array get_chase_trail();
	bool check_actor_status(NodePath ent_path);

	// COMBAT ---------------------------------------
	// Health Management
	int get_health();
	void set_health(int new_health);
	bool add_health(int amount);
	int get_health_max();
	int get_armor();
	void set_armor(int new_armor);
	bool add_armor(int amount);
	int get_armor_max();
	void set_armor_max(int new_armor_max);
	float get_armor_rating();
	void set_armor_rating(float new_armor_rating = 0.666f);
	// Powerup Management
	float get_superdamage();
	// Damage
	void damage(int amount, Node* attack = nullptr, NodePath attacker = NodePath());
	void knockback(Vector3 dir, float power = 1.0f);
	void popup(float power = 1.0f);
	float get_shielding();
	void bleed(Transform hit_xform);
	//void blood_splat(int dmg);
	int get_bleed_type();
	Spatial* get_gib(int gib_index);
	void gib(float power = 0.5f, bool erase = true);
	int get_instagib();
	bool is_gibbed();
	void set_grabbed_by(NodePath g = NodePath()); NodePath get_grabbed_by();
	bool check_grabbed();
	void drop_armorshards(int amount = 5);

	// MONSTER AI -----------------------------------
	int build_enemy_list(int max_ents, float dist = 32.0f);
	bool sort_by_distance(Variant a, Variant b);
	void _enemy_found(Spatial* new_enemy);
	Spatial* enemy_search(float fov = 0.0f);
	void set_aim_queued(bool q);
	void queue_enemy_search(float fov = -1.1f);
	bool check_enemy_status();
	void clear_enemy();
	void _heard_player(Vector3 pos);
	float enemy_distance();
	bool enemy_in_range(float check_dist);
	Vector3 lazy_aim(Vector3 pos);
	Vector3 chase_check(float fov = -1.1f);
	void chase_enemy_walk(float delta, float fov = -1.1f, float turn_speed = 10.0f, bool ignore_floor = false);
	void _ai_routine(int flags);
	// Pathing
	void pathonce();
	void pathloop();
	void pathpong();

	// ANIMATION ------------------------------------
	void _enter_pvs();
	void _exit_pvs();
	bool is_in_pvs();
	void _anim_finished(String anim);

	// SOUND ----------------------------------------
	enum { CHAN_VOICE, CHAN_WEAPON, CHAN_BODY, CHAN_ITEM };
	void sfx_play(int chan, Ref<AudioStream> snd, int priority = 0);
	void sfx_set_vol(Node* chan, float new_vol);
	void sfx_silence();

	// SCRIPTING ------------------------------------
	void trigger(Node* caller);
	void set_think(String th, float n_th);
	void scripted_death();
	void scripted_gib();
	void silent_gib();
	void remove();
	void call_think();

	// SAVE DATA ------------------------------------
	Dictionary data_save();
	void data_load(Dictionary data);

	// STATE MANAGEMENT -----------------------------
	void state_enter();
	void state_idle(float delta);
	void state_physics(float delta);
	void state_change(int new_state);
	int get_current_state();
	
	// BASE PROCESSING ------------------------------
	void _init();
	void _ready();
	void _process(float delta);
	void _physics_process(float delta);
	void _exit_tree();
};
