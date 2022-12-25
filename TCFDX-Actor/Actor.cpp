/*******************************************************************************
ACTOR CLASS
Base class for all actor types, mainly players and monsters. Some types of
objects may use this class, like grenades, for the purposes of taking advantage
of the gravity and damage mechanics.

Node Tree Setup:
- KinematicBody "name"
	- CollisionShape "c"
	- Spatial "modelname"	(GLTF scene)
		- Spatial	(rig name)
			- Skeleton "Skeleton"
				-MeshInstance	(mesh name)
		- AnimationPlayer "AnimationPlayer"
	- AnimationPlayer "AnimationPlayer"
	- AudioStreamPlayer3D "sfx0"	(VOICE)
	- AudioStreamPlayer3D "sfx1"	(WEAPON)
	- AudioStreamPlayer3D "sfx2"	(ITEM)
	- AudioStreamPlayer3D "sfx3"	(BODY)
*******************************************************************************/
#include "Actor.h"

// GODOT ---------------------------------------------------------
void Actor::_register_methods()
{
	// Properties
	register_property("properties", &Actor::set_properties, &Actor::get_properties, {});
	register_property("spawnflags", &Actor::set_spawnflags, &Actor::get_spawnflags, (GameManager::FL_NOT_IN_DEATHMATCH+GameManager::FL_NOT_IN_TEAMDEATHMATCH));
	register_property("classname", &Actor::set_classname, &Actor::get_classname, String());
	register_property("trg_target", &Actor::set_trg_target, &Actor::get_trg_target, String());
	register_property("trg_targetfunc", &Actor::set_trg_targetfunc, &Actor::get_trg_targetfunc, String());
	register_property("trg_message", &Actor::set_trg_message, &Actor::get_trg_message, String());
	// Collision
	register_method("col_cast_motion", &Actor::col_cast_motion);
	register_method("col_set_solid", &Actor::col_set_solid);
	register_method("col_set_dead", &Actor::col_set_dead);
	register_method("set_noclip", &Actor::set_noclip);
	register_method("get_noclip", &Actor::get_noclip);
	// Navigation
	register_method("nav_floor_update", &Actor::nav_floor_update);
	register_method("get_nav_floor", &Actor::get_nav_floor);
	register_method("has_nav_floor", &Actor::has_nav_floor);
	register_method("teleport", &Actor::teleport);
	register_method("grav_set", &Actor::grav_set);
	register_method("grav_set_dir", &Actor::grav_set_dir);
	register_method("set_grav_dir", &Actor::set_grav_dir);
	register_method("get_grav_dir", &Actor::get_grav_dir);
	register_method("set_velocity", &Actor::set_velocity);
	register_method("set_velocity_local", &Actor::set_velocity_local);
	register_method("get_velocity", &Actor::get_velocity);
	register_method("enter_water", &Actor::enter_water);
	register_method("exit_water", &Actor::exit_water);
	register_method("sv_speed", &Actor::sv_speed);
	register_method("sv_friction", &Actor::sv_friction);
	register_method("sv_jump", &Actor::sv_jump);
	register_method("sv_weight", &Actor::sv_weight);
	register_method("set_flying", &Actor::set_flying);
	register_method("get_flying", &Actor::get_flying);
	register_method("set_friction_delay", &Actor::set_friction_delay);
	// Combat
	register_method("get_health", &Actor::get_health);
	register_method("set_health", &Actor::set_health);
	register_method("set_armor", &Actor::set_armor);
	register_method("damage", &Actor::damage);
	register_method("knockback", &Actor::knockback);
	register_method("popup", &Actor::popup);
	register_method("get_shielding", &Actor::get_shielding);
	register_method("bleed", &Actor::bleed);
	register_method("get_bleed_type", &Actor::get_bleed_type);
	register_method("gib", &Actor::gib);
	register_method("get_instagib", &Actor::get_instagib);
	register_method("is_gibbed", &Actor::is_gibbed);
	register_method("set_grabbed_by", &Actor::set_grabbed_by);
	register_method("get_grabbed_by", &Actor::get_grabbed_by);
	register_method("get_superdamage", &Actor::get_superdamage);
	register_method("drop_armorshards", &Actor::drop_armorshards);
	// Monster Ai
	register_method("build_enemy_list", &Actor::build_enemy_list);
	register_method("_enemy_found", &Actor::_enemy_found);
	register_method("_heard_player", &Actor::_heard_player);
	register_method("enemy_search", &Actor::enemy_search);
	register_method("set_aim_queued", &Actor::set_aim_queued);
	register_method("queue_enemy_search", &Actor::queue_enemy_search);
	register_method("pathonce", &Actor::pathonce);
	register_method("pathloop", &Actor::pathloop);
	register_method("pathpong", &Actor::pathpong);
	register_method("_ai_routine", &Actor::_ai_routine);
	// Animation
	register_method("_enter_pvs", &Actor::_enter_pvs);
	register_method("_exit_pvs", &Actor::_exit_pvs);
	register_method("is_in_pvs", &Actor::is_in_pvs);
	register_method("_anim_finished", &Actor::_anim_finished);
	// Audio
	register_method("sfx_play", &Actor::sfx_play);
	// Scripting
	register_method("trigger", &Actor::trigger);
	register_method("call_think", &Actor::call_think);
	register_method("scripted_death", &Actor::scripted_death);
	register_method("scripted_gib", &Actor::scripted_gib);
	register_method("silent_gib", &Actor::silent_gib);
	register_method("set_move_input", &Actor::set_move_input);
	register_method("get_move_input", &Actor::get_move_input);
	register_method("set_jump", &Actor::set_jump);
	register_method("remove", &Actor::remove);
	// Save Data
	register_method("data_save", &Actor::data_save);
	register_method("data_load", &Actor::data_load);
	// State Management
	register_method("get_current_state", &Actor::get_current_state);
	register_method("state_enter", &Actor::state_enter);
	register_method("state_idle", &Actor::state_idle);
	register_method("state_physics", &Actor::state_physics);
	register_method("state_change", &Actor::state_change);
	// Base Processing
	register_method("_ready", &Actor::_ready);
	register_method("_process", &Actor::_process);
	register_method("_physics_process", &Actor::_physics_process);
	register_method("_exit_tree", &Actor::_exit_tree);
	// Signals
	register_signal<Actor>("enemy_found", "new_enemy", GODOT_VARIANT_TYPE_OBJECT);
	register_signal<Actor>("just_landed");
	register_signal<Actor>("collision_changed", "new_layer", GODOT_VARIANT_TYPE_INT);
	register_signal<Actor>("actor_removed", "actor_path", GODOT_VARIANT_TYPE_NODE_PATH);
}

// QODOT ---------------------------------------------------------
void Actor::set_properties(Dictionary new_properties)
{
	properties = new_properties;
	if (!Engine::get_singleton()->is_editor_hint())
		return;
	if (properties.has("classname"))
		classname = properties["classname"];
	set_rotation_degrees(GameManager::demangler(properties));
	if (properties.has("spawnflags"))
		spawnflags = properties["spawnflags"];
	else
		spawnflags = GameManager::FL_NOT_IN_DEATHMATCH + GameManager::FL_NOT_IN_TEAMDEATHMATCH;
	if (properties.has("target"))
		trg_target = properties["target"];
	if (properties.has("targetfunc"))
		trg_targetfunc = properties["targetfunc"];
	if (properties.has("message"))
		trg_message = properties["message"];
}

Dictionary Actor::get_properties() { return properties; }
void Actor::set_classname(String n) { classname = n; } String Actor::get_classname() { return classname; }
void Actor::set_spawnflags(int x) { spawnflags = x; } int Actor::get_spawnflags() {	return spawnflags; }
void Actor::set_trg_target(String t) { trg_target = t; } String Actor::get_trg_target() { return trg_target; }
void Actor::set_trg_targetfunc(String f) { trg_targetfunc = f; } String Actor::get_trg_targetfunc() { return trg_targetfunc; }
void Actor::set_trg_message(String m) { trg_message = m; } String Actor::get_trg_message() { return trg_message; }

// COLLISION -----------------------------------------------------
float Actor::get_col_floor() { return col_floor; }
float Actor::get_col_radius() {	return col_radius; }

// Raycasting, useful for everything
Dictionary Actor::col_ray(Vector3 origin, Vector3 cast_to, int mask, Array exclude)
{
	return space_state->intersect_ray(origin, cast_to, exclude, mask, true, true);
}

Dictionary Actor::col_ray_body(Vector3 origin, Vector3 cast_to, int mask, Array exclude)
{
	return space_state->intersect_ray(origin, cast_to, exclude, mask, true, false);
}

Dictionary Actor::col_ray_area(Vector3 origin, Vector3 cast_to, int mask, Array exclude)
{
	return space_state->intersect_ray(origin, cast_to, exclude, mask, false, true);
}

// Shape checking, useful for telefrags among other things
Ref<PhysicsShapeQueryParameters> Actor::col_make_shape_query(Transform shape_transform, float shape_margin, int mask, Array exclude)
{
	Ref<PhysicsShapeQueryParameters> query = Ref<PhysicsShapeQueryParameters>(PhysicsShapeQueryParameters::_new());
	query->set_shape_rid(col_shape->get_rid());
	shape_transform *= col_node->get_transform();
	query->set_transform(shape_transform);
	query->set_margin(shape_margin);
	query->set_collision_mask(mask);
	query->set_exclude(exclude);
	return query;
}

Array Actor::col_shape_check(Transform shape_transform, float shape_margin, int mask, Array exclude)
{
	Ref<PhysicsShapeQueryParameters> query = col_make_shape_query(shape_transform, shape_margin, mask, exclude);
	query->set_collide_with_areas(true);
	return space_state->intersect_shape(query);
}

Array Actor::col_shape_check_body(Transform shape_transform, float shape_margin, int mask, Array exclude)
{
	Ref<PhysicsShapeQueryParameters> query = col_make_shape_query(shape_transform, shape_margin, mask, exclude);
	return space_state->intersect_shape(query);
}

Array Actor::col_shape_check_area(Transform shape_transform, float shape_margin, int mask, Array exclude)
{
	Ref<PhysicsShapeQueryParameters> query = col_make_shape_query(shape_transform, shape_margin, mask, exclude);
	query->set_collide_with_bodies(false);
	query->set_collide_with_areas(true);
	return space_state->intersect_shape(query);
}

Array Actor::col_cast_motion(float shape_margin, int mask, Array exclude, Vector3 motion)
{
	Ref<PhysicsShapeQueryParameters> query = col_make_shape_query(get_global_transform(), shape_margin, mask, exclude);
	return space_state->cast_motion(query, motion);
}

// Collision layer setting
void Actor::col_set_solid()
{
	int layers = GameManager::ACTOR_LAYER + GameManager::TRIGGER_LAYER;
	if (is_in_group("PLAYER") == false)
		layers += GameManager::AI_LAYER;
	emit_signal("collision_changed",GameManager::ACTOR_LAYER);
	set_collision_layer(layers);
	layers = GameManager::SOLID_LAYER;
	set_collision_mask(layers);
}

void Actor::col_set_dead()
{
	int layers = GameManager::DEAD_LAYER * int(!gibbed);
	emit_signal("collision_changed",layers);
	set_collision_layer(layers);
	layers = GameManager::MAP_LAYER;
	set_collision_mask(layers);
}

void Actor::set_noclip(bool is_noclip)
{
	if (is_noclip)
	{
		set_collision_layer(0);
		set_collision_mask(0);
		flying = true;
	}
	else
	{
		call("col_set_solid");
		flying = false;
	}
}

bool Actor::get_noclip()
{
	if (get_collision_layer() == 0 && get_collision_mask() == 0 && flying)
		return true;
	return false;
}

// NAVIGATION -----------------------------------
void Actor::nav_floor_update()
{
	Transform t = get_global_transform();
	Dictionary new_nav_floor = col_ray_body(t.origin, t.origin + grav_dir * (col_floor + col_radius),GameManager::MAP_LAYER,col_ex_self);
	if (!new_nav_floor.empty() && nav_floor.empty())
		emit_signal("just_landed");
	nav_floor = new_nav_floor;
}

Dictionary Actor::get_nav_floor() { return nav_floor; }

bool Actor::has_nav_floor() { return !nav_floor.empty(); }

bool Actor::nav_grav_dir()
{
	if (!nav_floor.empty())
	{
		Vector3 n = nav_floor["normal"];
		n = -n;
		Node* c = nav_floor["collider"];
		int grav_type = c->get("grav_type");
		switch (grav_type)
		{
		case GameManager::GRV_KEEP:
			return true;
		case GameManager::GRV_SET:
			grav_dir = n;
			return true;
		case GameManager::GRV_FLIP:
			grav_dir = nav_floor["normal"];
			return true;
		default:
			{
				float a = grav_dir.dot(n);
				if (a >= 0.5f && a < 0.99f)
				{
					grav_dir = n;
					return true;
				};
			};
		};
	};
	return false;
}

void Actor::nav_xform(float delta)
{
	Transform t = get_global_transform();
	Transform old_t = t;
	if (t.basis.y.distance_squared_to(-grav_dir) < 0.00001f)
		return;
	t.basis.y = -grav_dir;
	t.basis.x = -t.basis.z.cross(t.basis.y);
	t.basis.z = t.basis.x.cross(t.basis.y);
	t.basis.orthonormalize();
	if (delta > 0.0f)
	{
		float rot_spd = Math::lerp(5.0f, 7.5f, (velocity - grav_vector).length() / max_speed) * delta;
		set_global_transform(old_t.interpolate_with(t, rot_spd));
		return;
	}
	set_global_transform(t);
}

void Actor::nav_grav_accel(float delta)
{
	Vector3 prev_grav_vec = grav_vector;
	if (on_floor == false && grabbed_by.is_empty())
	{
		//if (grav_vector.length() > max_fall_speed)
		//	grav_vector = grav_dir * max_fall_speed;
		//else
		grav_vector += grav_dir * GAME->get_gravity() * delta;
		grav_accel = grav_vector - prev_grav_vec;
	}
	else
	{
		grav_vector *= 0.0f;
		grav_accel = grav_vector;
	};
}

void Actor::nav_set_direction(Basis basis_dir, Vector3 move_dir)
{
	Vector3 d;
	d = basis_dir.z * move_dir.z;
	d += basis_dir.x * move_dir.x;
	d += basis_dir.y * move_dir.y;
	nav_dir = d.normalized();
}

Vector3 Actor::nav_friction(Vector3 vel, float delta)
{
	if (water_jump_delay > 0.0f)
		return vel;
	// Make bunny hopping easier; set in nav_jump
	if (on_floor && friction_delay > 0.0f)
	{
		friction_delay = fmaxf(friction_delay - delta, 0.0f);
		return vel;
	}
	// Apply friction
	vel -= grav_vector;
	float cur_spd = vel.length();
	if (cur_spd < 0.0625f)
		return grav_vector;
	float frc = 0.0f;
	// Water friction
	if (water_level >= 2)
		frc = cur_spd * water_friction * water_level * delta;
	// Ground friction
	else if (on_floor || flying)
	{
		frc = fmaxf(cur_spd, stop_speed) * friction * delta;
		if (!check_bottom && !flying)
			frc *= 2.0f;
	}
	if (frc > 0.0f)
	{
		if (cur_spd == 0.0f)
			cur_spd = 0.01f;
		return vel * fmaxf(cur_spd - frc, 0.0f) / cur_spd + grav_vector;
	}
	return vel + grav_vector;
}

Vector3 Actor::nav_accelerate(Vector3 vel, float delta)
{
	vel -= grav_vector;
	float wish_spd = nav_dir.length() * max_speed;
	float add_spd = wish_spd - vel.dot(nav_dir);
	if (add_spd <= 0.0)
		return vel + grav_vector;
	float acc;
	// Ground acceleration
	if (water_level < 2)
		acc = fminf(acceleration * delta * wish_spd, add_spd);
	// Swimming acceleration
	else
		acc = fminf(water_acceleration * delta * wish_spd * 0.7f, add_spd);
	return vel + nav_dir * acc + grav_vector;
}

Vector3 Actor::nav_air_accelerate(Vector3 vel, float delta)
{
	vel -= grav_vector;
	float wish_spd = nav_dir.length() * max_speed;
	float add_spd = fmaxf(wish_spd, 1.875f) - vel.dot(nav_dir);
	if (add_spd <= 0.0)
		return vel + grav_vector;
	float acc = fminf(air_acceleration * delta * wish_spd, add_spd);
	return vel + nav_dir * acc + grav_vector;
}

Vector3 Actor::nav_jump(Vector3 vel, float delta)
{
	/*if (water_jump_delay > 0.0f)
	{
		water_jump_delay -= delta;
		return vel;
	};*/
	if (water_level >= 2 || flying)
	{
		if (move_input.y != 0)
		{
			float water_jump_str = 3.125f;
			if (water_type == GameManager::SLIME)
				water_jump_str = 2.5f;
			else if (water_type == GameManager::LAVA)
				water_jump_str = 1.5625f;
			grav_vector *= 0.0f;
			return vel - grav_dir * water_jump_str * move_input.y;
		};
	}
	else if (jumping)
	{
		jumping = false;
		if (on_floor)
		{
			on_floor = false;
			friction_delay = 0.1f;
			grav_vector *= 0.0f;
			return vel - grav_dir * jump_strength;
		};
	};
	return vel;
}

void Actor::nav_move(float delta)
{
	Vector3 v = velocity;
	if (grabbed_by.is_empty())
	{
		v = nav_friction(v, delta);
		if (on_floor || water_level >= 2)
			v = nav_accelerate(v, delta);
		else
			v = nav_air_accelerate(v, delta);
		v += grav_accel;
		v = nav_jump(v, delta);
	};
	if (on_floor)
		velocity = move_and_slide_with_snap(v, grav_dir * col_floor, -grav_dir, false, 4, 0.785398f, false);
	else
		velocity = move_and_slide(v, -grav_dir, false, 4, 0.785398f, false);
	on_floor = is_on_floor();
}

void Actor::nav_fly_move(float delta)
{
	// Sink or swim
	if (nav_dir.length() > 0.0f)
	{
		grav_vector *= 0.0f;
		grav_accel = -grav_dir * move_input.y * 0.03125f * delta;
	}
	else if (flying)
		grav_accel *= 0.0f;
	else
	{
		grav_accel = 1.875 * grav_dir * delta;
		if (on_floor)
		{
			grav_vector *= 0.0f;
			grav_accel = grav_dir * 0.01f;
		};
	};
	grav_vector += grav_accel;
	Vector3 v = velocity;
	if (grabbed_by.is_empty())
	{
		v = nav_friction(v, delta);
		v = nav_accelerate(v, delta);
		v += grav_accel;
		v = nav_jump(v, delta);
		if (flying && v.length() > max_speed)
			v = v.normalized() * max_speed;
	};
	velocity = move_and_slide(v, -grav_dir, false, 4, 0.7853982f, false);
	on_floor = is_on_floor();
}

bool Actor::nav_check_bottom(Vector3 offset)
{
	Dictionary c;
	Vector3 v;
	Basis b = get_global_transform().basis;
	for (int i = 0; i < 4; i++)
	{
		switch (i)
		{
		case 0:
			v = b.x + b.z;
			break;
		case 1:
			v = b.x - b.z;
			break;
		case 2:
			v = -b.x - b.z;
			break;
		case 3:
			v = -b.x + b.z;
			break;
		};
		v = v * col_radius + get_global_translation() + offset;
		c = col_ray_body(get_global_translation(), v - b.y * col_floor - b.y, GameManager::MAP_LAYER, col_ex_self);
		if (c.empty())
			return false;
	};
	return true;
}

bool Actor::nav_check_move(Vector3 offset)
{
	Ref<KinematicCollision> c = move_and_collide(offset, true, true, true);
	if (!c.is_null())
		if (cast_to<Node>(Ref<KinematicCollision>(c)->get_collider())->is_in_group("WORLD"))
			return false;
	return true;
}

Vector3 Actor::get_move_vec()
{
	return velocity + grav_dir * velocity;
}

void Actor::set_move_input(Vector3 new_move) { move_input = new_move; }
Vector3 Actor::get_move_input() { return move_input; }
void Actor::set_jump(bool j) { jumping = j; }

void Actor::teleport(Transform dest_xform)
{
	// Make sure our destination transform is a global_transform, not local!
	// Telefog at exit and entrance
	Spatial* fx = GAME->get_telefog();
	get_parent()->add_child(fx);
	fx->set_global_transform(get_global_transform());
	fx = GAME->get_telefog();
	get_parent()->add_child(fx);
	fx->set_global_transform(dest_xform);
	// Telefrag
	if (health > 0)
	{
		Array tfrag = col_shape_check_body(dest_xform, 0.05f, GameManager::ACTOR_LAYER, col_ex_self);
		for (int i = 0; i < tfrag.size(); i++)
		{
			Dictionary c = tfrag[i];
			Spatial* a = cast_to<Spatial>(c["collider"]);
			if (a->has_method("damage"))
			{
				if ((!is_in_group("PLAYER") && a->is_in_group("PLAYER")))// || a->is_in_group("ELDERGOD"))
				{
					set_collision_layer(0);
					set_collision_mask(GameManager::MAP_LAYER);
					call_deferred("damage", 100000, a, a);
				}
				else
				{
					a->set("collision_layer", 0);
					a->set("collision_mask", GameManager::MAP_LAYER);
					a->call_deferred("damage", 100000, this, this);
				};
			};
		};
	};
	// Teleport
	velocity = -dest_xform.basis.z * (velocity - grav_vector).length();
	grav_dir = -dest_xform.basis.y;
	grav_vector = grav_vector.length() * grav_dir;
	velocity += grav_vector;
	set_global_transform(dest_xform);
}

void Actor::grav_set(Transform xform)
{
	xform.orthonormalize();
	get_global_transform().set_basis(xform.basis);
	grav_dir = -xform.basis.y;
	nav_xform();
}

void Actor::grav_set_dir(Vector3 new_grav_dir)
{
	grav_dir = new_grav_dir;
	Transform t = get_global_transform();
	t.basis.y = -grav_dir;
	t.basis.x = -t.basis.z.cross(-grav_dir);
	if (t.basis.x.length() < 0.0001f)
	{
		t.basis.x = -t.basis.z.cross(get_global_transform().basis.y);
		if (t.basis.x.length() < 0.0001f)
			t.basis.x = -t.basis.z.cross(get_global_transform().basis.x);
	};
	t.basis.orthonormalize();
	set_global_transform(t);
}

void Actor::set_grav_dir(Vector3 new_grav_dir) { grav_dir = new_grav_dir; }
Vector3 Actor::get_grav_dir() { return grav_dir; }

void Actor::set_velocity(Vector3 v) { velocity = v; }
Vector3 Actor::get_velocity() { return velocity; };

void Actor::set_velocity_local(Vector3 v)
{
	velocity = to_global(v) - get_global_translation();
}

void Actor::face_pos(Vector3 tgt_pos, float turn_speed)
{
	Transform t = get_global_transform().looking_at(tgt_pos, -grav_dir);
	if (turn_speed > 0.0f)
		set_global_transform(get_global_transform().interpolate_with(t, turn_speed));
	else
		set_global_transform(t);
}

void Actor::enter_water(Area* water)
{
	if (water_level < 1)
	{
		if (GAME->get_time() > 0.1f)
			SND->play3d(sfx[CHAN_BODY], SND->S_WATER_ENTER, 50);
		water_level = 2;
		velocity *= 0.2f;
		grav_vector *= 0.0f;
	};
	water_vol = water;
}

void Actor::exit_water(Area* water)
{
	if (water_vol == water)
	{
		water_level = 0;
		velocity *= 0.5f;
		SND->play3d(sfx[CHAN_BODY], SND->S_WATER_EXIT, 50);
	}
}

void Actor::sv_speed(float new_spd) { run_speed = new_spd; }
void Actor::sv_friction(float new_frc) { friction = new_frc; }
void Actor::sv_jump(float new_jmp) { jump_strength = new_jmp; }
void Actor::sv_weight(float new_wgt) { weight = new_wgt; }
void Actor::set_flying(bool is_flying) { flying = is_flying; }
bool Actor::get_flying() { return flying; }
void Actor::set_friction_delay(float delay) { friction_delay = delay; }

// TARGETING ------------------------------------
Vector3 Actor::get_pos_dir(Vector3 tgt_pos, Vector3 axis)
{
	Vector3 d = tgt_pos - get_global_translation();
	Vector3 p = d.project(axis);
	return d - p;
}

float Actor::get_pos_dist(Vector3 tgt_pos, Vector3 axis)
{
	Vector3 d = tgt_pos - get_global_translation();
	Vector3 p = d.project(axis);
	return d.distance_squared_to(p);
}

void Actor::turn_towards_pos(float delta, Vector3 tgt_pos, float turn_speed)
{
	Basis b = get_global_transform().basis;
	Vector3 tgt_dir = get_pos_dir(tgt_pos, b.y);
	if (tgt_dir.length() < col_radius)
		return;
	tgt_dir.normalize();
	b.y.normalize();
	float rot_amount = -b.z.dot(tgt_dir);
	if (rot_amount > 0.999)
		return;
	if (rot_amount <= -1.0f)
		rot_amount = 3.141592f;
	else
		rot_amount = acos(rot_amount);
	rot_amount *= Math::sign(-b.z.rotated(b.y, 1.570796f).dot(tgt_dir));
	if (turn_speed > 0.0f)
		rotate(b.y, rot_amount * turn_speed * delta);
	else
		rotate(b.y, rot_amount);
}

bool Actor::line_of_sight(Vector3 tgt_pos, float fov)
{
	Vector3 view_vec = -get_global_transform().basis.z;
	Vector3 tgt = (tgt_pos - get_global_translation()).normalized();
	float a = view_vec.dot(tgt);
	if (a >= fov)
		return true;
	return false;
}

void Actor::chase_add_breadcrumb(float spacing)
{
	if (chase_trail.size() > 0 && get_global_translation().distance_squared_to(chase_trail.front()) < spacing)
		return;
	if (chase_trail.size() > 29)
		chase_trail.pop_front();
	chase_trail.push_back(get_global_translation());
}

Array Actor::get_chase_trail()
{
	return chase_trail;
}

bool Actor::check_actor_status(NodePath ent_path)
{
	if (ent_path.is_empty())
		return false;
	if (has_node(ent_path))
	{
		Node* ent = get_node(ent_path);
		if (ent->is_in_group("ACTOR"))
		{
			if (int(ent->call("get_health")) > 0)
				return true;
		};
	};
	return false;
}

// COMBAT ---------------------------------------
// Health Management
void Actor::set_health(int new_health) { health = new_health; }
int Actor::get_health() { return health; }

bool Actor::add_health(int amount)
{
	if (amount > 0 && health < health_max)
	{
		health += amount;
		if (health > health_max)
			health = health_max;
		return true;
	};
	return false;
}

int Actor::get_health_max() { return health_max; }

int Actor::get_armor() { return armor; }

void Actor::set_armor(int new_armor) { armor = new_armor; }

bool Actor::add_armor(int amount)
{
	if (amount > 0 && armor < armor_max)
	{
		armor += amount;
		if (armor > armor_max)
			armor = armor_max;
		return true;
	};
	return false;
}

void Actor::set_armor_max(int new_armor_max)
{
	if (new_armor_max >= 0)
		armor_max = new_armor_max;
}

int Actor::get_armor_max() { return armor_max; }
void Actor::set_armor_rating(float new_armor_rating) { armor_rating = new_armor_rating; }
float Actor::get_armor_rating() { return armor_rating; }

// Powerup Management
float Actor::get_superdamage() { return superdamage; }

// Damage
void Actor::damage(int amount, Node* attack, NodePath attacker)
{
	if (GAME->get_instagib())
		health = gib_threshold * 2;
	if (has_node(attacker))
	{
		Node* a = get_node(attacker);
		if (a->is_in_group("ACTOR"))
		{
			if (a != this && a->get("classname") != classname)
			{
				_enemy_found(cast_to<Spatial>(a));
				mad = true;
			};
			if (float(a->call("get_superdamage")) > 0.0f)
				amount *= 5;
		};
		// Self damage is halved
		if (attacker == get_path())
			amount /= 2;
	};
	if (invincibility > 0.0f)
	{
		SND->play3d(sfx[CHAN_ITEM],SND->S_INVINCIBLITY[1]);
		amount *= 0;
	}
	damaged = 0.02f;
	// Check armor
	if (armor > 0)
	{
		armor -= int(ceilf(amount * armor_rating));
		health -= int(ceilf(amount * (1.0f - armor_rating)) - Math::min(armor, 0));
	}
	else
		health -= amount;
	// Blood splatter
	Dictionary c = col_ray_body(get_global_translation(), to_global(Vector3(0.0f, -col_floor - 10.0f, 0.0f) + GAME->rand_vec3() * col_radius), GameManager::MAP_LAYER, Array::make());
	if (!c.empty())
	{
		Spatial* b = GAME->get_blood_decal(bleed_type, amount > 25 ? 1 : 0);
		cast_to<Node>(c["collider"])->add_child(b);
		b->set_global_translation(c["position"]);
		b->look_at((Vector3)c["position"] + (Vector3)c["normal"], b->to_global(Vector3::UP));
	};
	// Chance to hit the paint state
	if (current_state != ST_PAIN && health > 0 && rng->randi() % 100 < pain_chance)
		state_change(ST_PAIN);
	// Gibbing
	else if (health <= gib_threshold && !gibbed)
	{
		call("gib", float(abs(health)), true);
		gibbed = true;
	};
}

void Actor::knockback(Vector3 dir, float power)
{
	velocity += dir * (power / fmaxf(weight, 0.1f));
}

void Actor::popup(float power)
{
	velocity -= grav_vector + grav_dir * (power / fmaxf(weight, 0.1f));
	grav_vector *= 0.0f;
}

float Actor::get_shielding() { return shielding; }

void Actor::bleed(Transform hit_xform)
{
	Spatial* fx = GAME->get_bleed(bleed_type);
	get_parent()->add_child(fx);
	fx->set_global_transform(Transform(hit_xform.basis,fx->to_local(hit_xform.origin)));
}

int Actor::get_bleed_type() { return bleed_type; }

Spatial* Actor::get_gib(int gib_index)
{
	if (gib_index >= 0 && gib_index < gib_res.size())
		return cast_to<Spatial>(gib_res[gib_index]->instance());
	else
		return GAME->get_gib(bleed_type);
}

void Actor::gib(float power, bool erase)
{
	if (Engine::get_singleton()->is_editor_hint())
		return;
	emit_signal("collision_changed", 0);
	set_collision_layer(0);
	set_collision_mask(GameManager::MAP_LAYER);
	velocity *= 0.0f;
	grav_vector *= 0.0f;
	hide();
	anim_player->stop();
	for (int i = 0; i < 4; i++)
		sfx[i]->stop();
	SND->play3d(sfx[CHAN_BODY], SND->S_GIB, 666, 3.0);
	// Blood splatter
	Dictionary c = col_ray_body(get_global_translation(), to_global(Vector3(0.0f, -col_floor - 10.0f, 0.0f)), GameManager::MAP_LAYER, Array::make());
	if (!c.empty())
	{
		Spatial* b = GAME->get_blood_decal(bleed_type, 1);
		cast_to<Node>(c["collider"])->add_child(b);
		b->set_global_translation(c["position"]);
		b->look_at((Vector3)c["position"] + (Vector3)c["normal"], b->to_global(Vector3::UP));
		b->remove_from_group("BLOOD_DECAL");
	};
	// GIBS!
	Spatial* gib;
	for (int i = 0; i < int(Math::clamp(4.0f * weight, 8.0f, 30.0f)); i++)
	{
		if (i < gib_res.size())
			gib = get_gib(i);
		else if (i == gib_res.size())
			gib = GAME->get_blood_exp(bleed_type);
		else
			gib = GAME->get_gib(bleed_type);
		if (gib->is_in_group("GIB"))
		{
			Gib* g = cast_to<Gib>(gib);
			g->grav_dir = grav_dir;
			g->power = fminf(power, 99.0f);
			if (!erase)
				g->set_erase(false);
			g->erase_ct = 15.0f + rng->randf() * 5.0f;
			g->add_to_group("SAV");
			g->set_bleed_type(bleed_type);
		}
		get_parent()->add_child(gib);
		gib->set_global_transform(get_global_transform());
	};
}

int Actor::get_instagib() { return health_max - gib_threshold; }

bool Actor::is_gibbed() { return gibbed; }

void Actor::set_grabbed_by(NodePath g)
{
	if (!has_node(g) || g == get_path())
		g = NodePath();
	grabbed_by = g;
}

NodePath Actor::get_grabbed_by()
{
	if (!has_node(grabbed_by))
		grabbed_by = NodePath();
	return grabbed_by;
}

bool Actor::check_grabbed()
{
	if (grabbed_by == get_path())
		grabbed_by = NodePath();
		return false;
	if (check_actor_status(grabbed_by))
		return true;
	grabbed_by = NodePath();
	return false;
}

void Actor::drop_armorshards(int amount)
{
	if (!Engine::get_singleton()->is_editor_hint())
	{
		amount = (GAME->get_difficulty() > 0) ? int(amount / (GAME->get_difficulty() + 1)) : amount;
		if (amount > 0 && GAME->get_difficulty() < GameManager::NIGHTMARE)
			GAME->spawn_armorshards(this, amount);
	};
}

// MONSTER AI -----------------------------------
int Actor::build_enemy_list(int max_ents, float dist)
{
	if (actorflags &= GameManager::FL_PLAYER)
		return 0;
	SceneTree* tree = get_tree();
	Array nodes_in_group;
	Vector3 o = get_global_translation();
	dist *= dist;
	// Distances should not be squared!
	// We compare each component directly rather than measure the vector to vector length.
	//Vector3 omin = get_global_translation() - Vector3::ONE * dist;
	//Vector3 omax = get_global_translation() + Vector3::ONE * dist;

	if (max_ents > 100)
		max_ents = 100;
	int count = 0;

	
	for (int i = 0; i < enemy_groups.size(); i++)
	{
		nodes_in_group = tree->get_nodes_in_group(enemy_groups[i]);
		for (int j = 0; j < nodes_in_group.size(); j++)
		{
			Spatial* e = nodes_in_group[j];
			// Not allowed
			int e_flags = int(e->call("get_spawnflags"));
			if (e_flags & (GameManager::FL_DEAD | GameManager::FL_GIB))
				continue;

			// Out of range
			/*Vector3 e_pos = e->get_global_translation();
			if (e_pos.x < omin.x || e_pos.x > omax.x ||
				e_pos.y < omin.y || e_pos.y > omax.y ||
				e_pos.z < omin.z || e_pos.z > omax.z)*/
			if (e->get_global_translation().distance_squared_to(o) > dist)
				continue;
			
			enemies[count] = e;
			count++;
			if (count >= max_ents)
				return count;
		};
	};
	return count;
}

bool Actor::sort_by_distance(Variant a, Variant b)
{
	if (a.get_type() == Variant::NODE_PATH && b.get_type() == Variant::NODE_PATH)
	{
		Vector3 origin = get_global_translation();
		if (cast_to<Spatial>(get_node(a))->get_global_translation().distance_squared_to(origin) < cast_to<Spatial>(get_node(b))->get_global_translation().distance_squared_to(origin))
			return true;
	};
	return false;
}

void Actor::_heard_player(Vector3 pos)
{
	if (current_state != ST_TELESPAWN)
		if (mad == false && health > 0 && pos.distance_squared_to(get_global_translation()) < hearing_range)
			emit_signal("enemy_found", enemy_search(-1.0f));
}

bool Actor::check_enemy_status()
{
	if (enemy != nullptr && !enemy->is_queued_for_deletion() && GAME->get_notarget() == false)
		if (int(enemy->call("get_health")) > 0)
			return true;
	return false;
}

void Actor::clear_enemy()
{
	enemy = nullptr;
	enemy_path = NodePath();
}

void Actor::set_aim_queued(bool q) { aim_queued = q; }

// Add this Actor to the Ai Manager Targeting Queue; when their turn comes up, Ai Manager will have the Actor emit the "enemy_found" signal.
void Actor::queue_enemy_search(float fov)
{
	if (in_pvs && !aim_queued && queue_timer <= 0.0f)
	{
		queue_timer = rng->randf_range(0.05f, 0.2f);
		AIM->queue_enemy_search(get_path(), fov);
	};
}

Spatial* Actor::enemy_search(float fov)
{
	if (GAME->get_notarget() || spawnflags & GameManager::FL_DOCILE)
		return nullptr;
	// If we're already fighting someone, just stick with them.
	// Put it here because we want to be able to lose the target if they escape sight.
	if (check_enemy_status() && line_of_sight(last_enemy_pos, fov))
		return enemy;
	clear_enemy();
	Vector3 o = get_global_translation();
	float cr = powf(col_radius + 1.1f, 2.0f);
	int count = build_enemy_list(100, 32.0f);
	for (int i = 0; i < count; i++)
	{
		Spatial* e = enemies[i];
		if (e == nullptr || e->is_queued_for_deletion())
			continue;
		// Ignore the dead
		if (int(e->call("get_health")) <= 0)
			continue;
		Vector3 e_pos = e->get_global_translation();
		// Can we see the target?
		if (fov >= -1.0f)
		{
			if (line_of_sight(e_pos, fov) && col_ray(o, e_pos, GameManager::MAP_LAYER + GameManager::VIS_LAYER, col_ex_self).empty())
				return e;
		}
		// We don't care if we can see the target or not
		else
			return e;
		// Is the Actor watching their back?
		if (e_pos.distance_squared_to(o) < cr)
			return e;
	};
	// We didn't find any valid targets
	return nullptr;
}

void Actor::_enemy_found(Spatial* new_enemy)
{
	if (new_enemy == nullptr || (GAME->get_notarget() && new_enemy->is_in_group("PLAYER")))
	{
		enemy = nullptr;
		enemy_path = NodePath();
		return;
	};
	if (new_enemy->is_in_group("ACTOR"))
	{
		enemy_path = new_enemy->get_path();
		enemy = new_enemy;
		last_enemy_pos = enemy->get_global_translation();
		mad = true;
		if (has_method("snd_play_mad"))
			call("snd_play_mad");
		else if (!sfx[CHAN_VOICE]->is_playing() && !s_mad.empty())
			SND->play3d(sfx[CHAN_VOICE], s_mad[rng->randi()%s_mad.size()], 100, 3.0f);
	};
}

// Returns the square of the enemy's distance, since that's cheaper for performance
// Make sure to square all intended distances checking against this
float Actor::enemy_distance()
{
	if (enemy != nullptr)
		return enemy->get_global_translation().distance_squared_to(get_global_translation());
	return -1.0f;
}

bool Actor::enemy_in_range(float check_dist)
{
	if (enemy != nullptr)
	{
		float d = enemy->get_global_translation().distance_squared_to(get_global_translation());
		if (abs(check_dist) > d)
			return true;
	};
	return false;
}

Vector3 Actor::lazy_aim(Vector3 pos)
{
	pos = to_local(pos);
	pos.x = 0.0f;
	return to_global(pos);
}

// Cycle through the target's chase trail positions; tests if there is any map
// geometry blocking the Actor's line of sight to their target or chase trail
// Used for AI navigation, best paired with "last_enemy_pos"
Vector3 Actor::chase_check(float fov)
{
	if (enemy != nullptr)
	{
		bool in_fov = true;
		Vector3 e_pos = enemy->get_global_translation();
		if (fov > 0.0f)
			in_fov = line_of_sight(e_pos, fov);
		if (in_fov && col_ray(get_global_translation(), e_pos, GameManager::MAP_LAYER + GameManager::VIS_LAYER, col_ex_self).empty())
			return e_pos;
		Array trail = enemy->call("get_chase_trail");
		for (int i = 0; i < trail.size(); i++)
		{
			if (fov > 0.0 && line_of_sight(trail[i], fov) == false)
				continue;
			if (col_ray(get_global_translation(), trail[i], GameManager::MAP_LAYER + GameManager::VIS_LAYER, col_ex_self).empty())
				return trail[i];
		};
	};
	return get_global_translation();
}

void Actor::chase_enemy_walk(float delta, float fov, float turn_speed, bool ignore_floor)
{
	if (enemy == nullptr)
		return;
	if (hunt_time > 0.0f)
		hunt_time -= delta;
	Transform t = get_global_transform();
	Vector3 v = -t.basis.z * (max_speed * delta + col_radius);
	// Can we see where the enemy is or was?
	Vector3 new_enemy_pos = last_enemy_pos;
	if (hunt_time <= 0.0f)
	{
		new_enemy_pos = chase_check(fov);
		if (new_enemy_pos == t.origin)
		{
			Dictionary c = col_ray_body(t.origin, t.origin + v, GameManager::MAP_LAYER, col_ex_self);
			if (c.empty() == false)
				new_enemy_pos = Vector3(c["position"]) + v.bounce(Vector3(c["normal"])) * 30.0f;
			else
			{
				float ang = float(rng->randi() % 4) * 45.0f;
				new_enemy_pos = t.origin + v.rotated(t.basis.y, Math::deg2rad(ang)) * 30.0f;
			}
			hunt_time = 1.0f;
		};
	};
	// Don't walk off ledges; rely on triggers or custom Actor code to do so
	if (!ignore_floor && !stationary && !nav_check_bottom(v))
	{
		velocity = grav_vector;
		Vector3 v2;
		if (rng->randi() % 2 == 0)
		{
			for (float ang = 0.0f; ang <= 315.0f; ang += 45.0f)
			{
				v2 = v.rotated(t.basis.y, Math::deg2rad(ang));
				if (nav_check_bottom(v2))
				{
					hunt_time = 0.5f;
					new_enemy_pos = v2.normalized() * (col_radius * 30.0f);
				};
			};
		}
		else
		{
			for (float ang = 315.0f; ang >= 0.0f; ang -= 45.0f)
			{
				v2 = v.rotated(t.basis.y, Math::deg2rad(ang));
				if (nav_check_bottom(v2))
				{
					hunt_time = 0.5f;
					new_enemy_pos = v2.normalized() * (col_radius * 30.0f);
				};
			};
		};
	};
	// We found our enemy
	if (new_enemy_pos != t.origin)
		last_enemy_pos = new_enemy_pos;
	// Move towards the enemy position
	if (t.origin.distance_squared_to(last_enemy_pos) > powf(col_radius + 0.5f, 2.0f))
	{
		turn_towards_pos(delta, last_enemy_pos, turn_speed);
		move_input.z = -1.0f * !stationary;
	}
	else
		move_input.z = 0.0f;
}

// Pathing
void Actor::pathonce()
{
	if (!mad)
	{
		path_loop_type = ONCE;
		state_change(ST_PATHING);
	};
}

void Actor::pathloop()
{
	if (!mad)
	{
		path_loop_type = LOOP;
		state_change(ST_PATHING);
	};
}

void Actor::pathpong()
{
	if (!mad)
	{
		path_loop_type = PINGPONG;
		state_change(ST_PATHING);
	};
}

// When an NPC runs into an AI Trigger volume, run a routine
void Actor::_ai_routine(int flags)
{
	switch (flags)
	{
	case GameManager::AI_NOPASS: // NO_PASS
	{
		Transform t = get_global_transform();
		Vector3 v = velocity;
		velocity = grav_vector;
		if (rng->randi() % 2 == 0)
		{
			for (float ang = 0.0f; ang <= 315.0f; ang += 45.0f)
			{
				v = v.rotated(t.basis.y, Math::deg2rad(ang));
				if (nav_check_bottom(v))
				{
					hunt_time = 0.5f;
					last_enemy_pos = v.normalized() * (col_radius * 30.0f);
				};
			};
		}
		else
		{
			for (float ang = 315.0f; ang >= 0.0f; ang -= 45.0f)
			{
				v = v.rotated(t.basis.y, Math::deg2rad(ang));
				if (nav_check_bottom(v))
				{
					hunt_time = 0.5f;
					last_enemy_pos = v.normalized() * (col_radius * 30.0f);
				};
			};
		};
		turn_towards_pos(10.0f, last_enemy_pos);
	}
	case GameManager::AI_GIB:
		scripted_gib();
		return;
	default:
		return;
	};
}

// ANIMATION --------------------------------------
void Actor::_enter_pvs()
{
	in_pvs = true;
}

void Actor::_exit_pvs()
{
	in_pvs = false;
}

bool Actor::is_in_pvs() { return in_pvs; }

void Actor::_anim_finished(String anim)
{
	if (current_state == ST_DEAD)
		call("col_set_dead");
}

// SOUND ------------------------------------------
void Actor::sfx_play(int chan, Ref<AudioStream> snd, int priority)
{
	SND->play3d(sfx[chan], snd, priority);
}

void Actor::sfx_set_vol(Node* chan, float new_vol)
{
	if (chan->get_class() == "AudioStreamPlayer3D")
		cast_to<AudioStreamPlayer3D>(chan)->set_unit_db(Math::linear2db(new_vol));
	else if (chan->get_class() == "AudioStreamPlayer")
		cast_to<AudioStreamPlayer>(chan)->set_volume_db(Math::linear2db(new_vol));
}

void Actor::sfx_silence()
{
	for (int i = 0; i <= CHAN_ITEM; i++)
	{
		sfx_set_vol(sfx[i], 0.0f);
		sfx[i]->stop();
	};
}

// SCRIPTING ------------------------------------
void Actor::trigger(Node* caller)
{
	// The dead can no longer act
	if (current_state == ST_DEAD)
		return;
	// You can't trigger players, what's the matter with you?
	if (is_in_group("PLAYER"))
		return;
	// Telespawn enemies need to "warp" in
	if (current_state == ST_TELESPAWN)
	{
		state_change(ST_IDLE);
		show();
		col_set_solid();
		for (int i = 0; i < 4; i++)
			sfx_set_vol(sfx[i], 1.0f);
		teleport(get_global_transform());
	};
	if (spawnflags & GameManager::FL_DOCILE)
		spawnflags &= ~GameManager::FL_DOCILE;
	Spatial* ent = nullptr;
	if (caller != nullptr)
	{
		// Monster deaths act as triggers, so get whoever killed it
		if (caller->is_in_group("MONSTER"))
			ent = caller->get("enemy");
		// Trigger volumes will give us the last entity
		else if (caller->is_in_group("TRIGGER"))
		{
			Node* n = caller->call("get_last_entity");
			if (n->is_in_group("ACTOR"))
				ent = cast_to<Spatial>(n);
		};
	};
	// We can keep enemies still and quiet until the moment's right
	// Or we can interrupt their stroll / throes of worship
	if (current_state == ST_AMBUSH || current_state == ST_PATHING || current_state == ST_WORSHIP)
		state_change(ST_IDLE);
	// Add this guy to our potential targets list, no matter how far
	if (ent != nullptr && ent->is_in_group("ACTOR"))
	{
		for (int i = 0; i < enemy_groups.size(); i++)
			if (ent->is_in_group(enemy_groups[i]))
			{
				_enemy_found(ent);
				return;
			};
	}
	else
		emit_signal("enemy_found", enemy_search(0.0f));
	mad = true;
}

void Actor::set_think(String th, float n_th)
{
	think = th;
	next_think = GAME->get_time() + n_th;
	think_check = true;
}

void Actor::call_think()
{
	if (think_check == true && GAME->get_time() > next_think)
	{
		think_check = false;
		if (think == "gib")
			scripted_gib();
		else if (has_method(think))
			call(think);
		else if (think == "start")
		{
			if (spawnflags > 0)
			{
				if (spawnflags & GameManager::FL_GIB)
				{
					if (properties.has("spawnvar"))
						state_timer = properties["spawnvar"];
					else
						state_timer = 0.0f;
					state_change(ST_GIBSTART);
					return;
				};
				if (spawnflags & GameManager::FL_DEAD)
				{
					if (properties.has("spawnvar"))
						state_timer = properties["spawnvar"];
					else
						state_timer = -1.0f;
					state_change(ST_DEADSTART);
					return;
				};
				stationary = (spawnflags & GameManager::FL_STATIONARY);
				if (spawnflags & GameManager::FL_TELESPAWN)
					state_change(ST_TELESPAWN);
				if (spawnflags & GameManager::FL_AMBUSH)
					state_change(ST_AMBUSH);
				else if (spawnflags & GameManager::FL_PATHING)
				{
					stationary = false;
					if (properties.has("spawnvar"))
					{
						path_loop_type = properties["spawnvar"];
					}
					else
						path_loop_type = ONCE;
					state_change(ST_PATHING);
				};
			};
			if (current_state == ST_START)
				state_change(ST_IDLE);
		};
	};
}

void Actor::scripted_death()
{
	armor = 0;
	damage(health);
}

void Actor::scripted_gib()
{
	armor = 0;
	health = 0;
	call("gib",10.0f, false);
}

void Actor::silent_gib()
{
	armor = 0;
	health = 0;
	gib(10.0f, false);
	sfx_silence();
}

void Actor::remove()
{
	if (is_in_group("PLAYER"))
		return;
	current_state = ST_REMOVED;
	think_check = false;
	hide();
	sfx_silence();
	set_collision_layer(0);
	set_collision_mask(0);
	emit_signal("actor_removed", get_path());
}

// SAVE DATA ---------------------------------------
Dictionary Actor::data_save()
{
	Dictionary data;
	// State and Scripting
	data["spawnflags"] = spawnflags;
	data["current_state"] = current_state;
	data["previous_state"] = previous_state;
	data["state_timer"] = state_timer;
	data["think"] = think;
	data["next_think"] = next_think;
	data["think_check"] = think_check;
	// Navigation
	data["col_layer"] = get_collision_layer();
	data["col_mask"] = get_collision_mask();
	data["origin"] = get_translation();
	data["rotation"] = get_rotation();
	data["scale"] = get_scale();
	data["velocity"] = velocity;
	data["grav_dir"] = grav_dir;
	data["grav_vector"] = grav_vector;
	data["flying"] = flying;
	data["move_input"] = move_input;
	data["on_floor"] = on_floor;
	data["jumping"] = jumping;
	data["check_bottom"] = check_bottom;
	data["water_level"] = water_level;
	data["water_vol"] = water_vol;
	data["water_type"] = water_type;
	data["nav_dir"] = nav_dir;
	data["nav_target_pos"] = nav_target_pos;
	data["max_speed"] = max_speed;
	// Health
	data["health_max"] = health_max;
	data["health"] = health;
	data["armor_max"] = armor_max;
	data["armor"] = armor;
	data["armor_rating"] = armor_rating;
	// Combat
	data["attack_input"] = attack_input;
	data["shielding"] = shielding;
	data["superdamage"] = superdamage;
	data["invincibility"] = invincibility;
	data["gibbed"] = gibbed;
	data["grabbed_by"] = grabbed_by;
	// Monster Ai
	data["mad"] = mad;
	data["enemy_path"] = enemy_path;
	data["last_enemy_pos"] = last_enemy_pos;
	data["hunt_time"] = hunt_time;
	data["hearing_range"] = hearing_range;
	data["path_name"] = path_name;
	data["path_index"] = path_index;
	data["path_loop_type"] = path_loop_type;
	data["stationary"] = stationary;
	// Animation
	data["visible"] = is_visible();
	data["anim"] = anim_player->get_assigned_animation();
	data["anim_time"] = 3600.0f;
	if (anim_player->is_playing())
		data["anim_time"] = anim_player->get_current_animation_position();
	return data;
}

void Actor::data_load(Dictionary data)
{
	if (current_state == ST_REMOVED)
		return;
	// State and Scripting
	spawnflags = data["spawnflags"];
	current_state = data["current_state"];
	previous_state = data["previous_state"];
	state_timer = data["state_timer"];
	think = data["think"];
	next_think = data["next_think"];
	think_check = data["think_check"];
	// Navigation
	set_collision_layer(data["col_layer"]);
	set_collision_mask(data["col_mask"]);
	set_translation(GameManager::str_to_vec3(data["origin"]));
	set_rotation(GameManager::str_to_vec3(data["rotation"]));
	set_scale(GameManager::str_to_vec3(data["scale"]));
	velocity = GameManager::str_to_vec3(data["velocity"]);
	grav_dir = GameManager::str_to_vec3(data["grav_dir"]);
	grav_vector = GameManager::str_to_vec3(data["grav_vector"]);
	flying = data["flying"];
	move_input = GameManager::str_to_vec3(data["move_input"]);
	on_floor = data["on_floor"];
	jumping = data["jumping"];
	check_bottom = data["check_bottom"];
	water_level = data["water_level"];
	water_vol = data["water_vol"];
	water_type = data["water_type"];
	nav_dir = GameManager::str_to_vec3(data["nav_dir"]);
	nav_target_pos = GameManager::str_to_vec3(data["nav_target_pos"]);
	max_speed = data["max_speed"];
	// Health
	health_max = data["health_max"];
	health = data["health"];
	armor_max = data["armor_max"];
	armor = data["armor"];
	armor_rating = data["armor_rating"];
	// Combat
	attack_input = data["attack_input"];
	shielding = data["shielding"];
	superdamage = data["superdamage"];
	invincibility = data["invincibility"];
	gibbed = data["gibbed"];
	grabbed_by = data["grabbed_by"];
	// Monster Ai
	mad = data["mad"];
	enemy_path = data["enemy_path"];
	if (has_node(enemy_path))
	{
		enemy = cast_to<Spatial>(get_node(enemy_path));
	};
	last_enemy_pos = GameManager::str_to_vec3(data["last_enemy_pos"]);
	hunt_time = data["hunt_time"];
	hearing_range = data["hearing_range"];
	path_name = data["path_name"];
	if (current_state == ST_PATHING)
		state_enter();
	path_index = data["path_index"];
	path_loop_type = data["path_loop_type"];
	stationary = data["stationary"];
	// Animation
	set_visible(data["visible"]);
	anim_player->play(data["anim"]);
	anim_player->call_deferred("seek", data["anim_time"]);
	// Audio
	if (spawnflags & GameManager::FL_GIB)
	{
		for (int i = 0; i < 4; i++)
			sfx[i]->stop();
	};
}

// STATE MANAGEMENT -----------------------------
int Actor::get_current_state() { return current_state; }

void Actor::state_enter()
{
	Array local_var = {};
	switch (current_state)
	{
	case ST_IDLE:
		return;
	case ST_PAIN:
		move_input *= 0.0f;
		if (pain_anims.size() > 0)
		{
			int r = (int)rng->randi() % pain_anims.size();
			anim_player->play(pain_anims[r]);
		};
		if (has_method("snd_pain"))
			call("snd_pain");
		else if (!sfx[CHAN_VOICE]->is_playing() && !s_pain.empty())
			SND->play3d(sfx[CHAN_VOICE], s_pain[rng->randi() % s_pain.size()], 50, 3.0f);
		return;
	case ST_DEAD:
		if (previous_state != ST_DEAD)
		{
			if (trg_target != "")
			{
				GAME->trigger_target(this, trg_target);
				trg_target = "";
			}
			move_input *= 0.0f;
			if (gibbed == false)
			{
				if (death_anims.size() > 0)
				{
					int r = (int)rng->randi() % death_anims.size();
					anim_player->play(death_anims[r]);
				};
				if (previous_state != ST_DEADSTART)
				{
					if (has_method("snd_die"))
						call("snd_die");
					else if (!s_die.empty())
						SND->play3d(sfx[CHAN_VOICE], s_die[rng->randi() % s_die.size()], 100, 10.0f);
				};
			};
		}
		else
		{
			anim_player->stop();
			for (int i = 0; i <= CHAN_ITEM; i++)
				sfx[i]->stop();
		};
		return;
	case ST_PATHING:
	{
		path_list.clear();
		Array path_ents = get_tree()->get_nodes_in_group("path_" + trg_target);
		for (int i = 0; i < path_ents.size(); i++)
			path_list.push_back(path_ents[i]);
		if (path_list.size() > 0)
		{
			std::sort(path_list.begin(), path_list.end(), [](const PathDx* a, const PathDx* b) { return a->path_index < b->path_index; });
			Vector3 o = get_global_translation();
			PathDx* closest = path_list[0];
			PathDx* next_pt;
			for (int i = 0; i < path_list.size(); i++)
			{
				next_pt = path_list[i];
				if (o.distance_squared_to(next_pt->get_global_translation()) < o.distance_squared_to(closest->get_global_translation()))
					closest = next_pt;
			};
			path_index = closest->path_index;
		}
		else
			state_change(ST_IDLE);
		return;
	}
	case ST_DEADSTART:
	{
		String death_anim_override = "";
		if (state_timer >= 0.0f)
			death_anim_override = "die" + String::num(int(state_timer));
		sfx_set_vol(sfx[CHAN_VOICE], 0.0f);
		health = 0;
		state_change(ST_DEAD);
		call("col_set_dead");
		if (death_anim_override != "")
			anim_player->play(death_anim_override, -1.0f, 1.0f, true);
		else
			anim_player->play(anim_player->get_assigned_animation(), -1.0f, 1.0f, true);
		sfx_silence();
		return;
	}
	case ST_GIBSTART:
		set_think("silent_gib", state_timer);
		return;
	};
}

void Actor::state_idle(float delta)
{
	// Monster Ai
	switch (current_state)
	{
	case ST_IDLE:
		if (!is_in_group("PLAYER"))
		{
			if (mad)
				max_speed = run_speed;
			else
				max_speed = walk_speed;
		};
		break;
	case ST_PATHING:
	case ST_AMBUSH:
		if (damaged > 0.0f)
			state_change(ST_IDLE);
		break;
	};
	mad = check_enemy_status();
	if (health <= 0 && current_state != ST_DEAD)
		state_change(ST_DEAD);
	// We don't want big dudes to accidentally reteleport over and over again
	if (teleport_delay > 0)
		teleport_delay -= delta;
}

void Actor::state_physics(float delta)
{
	nav_floor_update();
	if (!grabbed_by.is_empty())
		check_grabbed();
	if (current_state == ST_PATHING)
	{
		if (path_list.size() > 0)
		{
			Vector3 path_pos = path_list[path_index]->get_global_translation();
			if (get_global_translation().distance_squared_to(path_pos) > powf(fmaxf(max_speed * delta, col_floor), 2.0f))
			{
				turn_towards_pos(delta, path_pos);
				move_input.z = -1.0f;
			}
			else
			{
				path_index += 1;
				if (path_index >= path_list.size())
				{
					if (path_loop_type == LOOP)
					{
						path_index = 0;
					}
					else if (path_loop_type == PINGPONG)
					{
						if (path_list[0]->path_index < path_list[path_list.size() - 1]->path_index)
							std::sort(path_list.begin(), path_list.end(), [](const PathDx* a, const PathDx* b) { return a->path_index > b->path_index; });
						else
							std::sort(path_list.begin(), path_list.end(), [](const PathDx* a, const PathDx* b) { return a->path_index < b->path_index; });
						path_index = 0;
					}
					else
					{
						move_input.z = 0.0f;
						state_change(ST_IDLE);
					};
				};
			};
		}
		else
		{
			move_input.z = 0.0f;
			state_change(ST_IDLE);
		};
	};
}

void Actor::state_change(int new_state)
{
	previous_state = current_state;
	current_state = new_state;
	call("state_exit");
	call("state_enter");
}

// BASE PROCESSING -----------------------------------------------
void Actor::_init()
{
	// Health management
	health_max = 100;
	health = health_max;
	armor_max = 0;
	armor = armor_max;
	// Combat
	if (!is_in_group("UNXC"))
		enemy_groups.push_back("UNXC");
	else
		enemy_groups.push_back("MONSTER");
	weight = 1.0f;
	pain_chance = -1;
	gib_threshold = -40;
	// Collision
	col_radius = 0.5f;
	col_floor = 1.0f;
	// Navigation
	walk_speed = 4.5f;
	run_speed = 10.0f;
	max_speed = walk_speed;
	stop_speed = 3.125f;
	acceleration = 10.0f;
	air_acceleration = 0.7f;
	water_acceleration = 10.0f;
	friction = 4.0f;
	water_friction = 4.0f;
	jump_strength = 8.4375f;
	max_fall_speed = 1.0f;
	// Animation
	pain_anims.push_back("pain");
	death_anims.push_back("die");
}

void Actor::_ready()
{
	if (!Engine::get_singleton()->is_editor_hint())
	{
		GAME = cast_to<GameManager>(get_node("/root/GameManager"));
		SND = cast_to<SoundManager>(get_node("/root/SoundManager"));
		AIM = cast_to<AiManager>(get_node("/root/AiManager"));
		rng = Ref<RandomNumberGenerator>(RandomNumberGenerator::_new());
		rng->set_seed(get_name().to_int());
		// Onready vars
		space_state = get_world()->get_direct_space_state();
		col_ex_self.append(this);
		anim_player = cast_to<AnimationPlayer>(get_node("AnimationPlayer"));
		for (int i = 0; i < 4; i++)
		{
			NodePath path = NodePath("sfx" + String::num(i));
			if (has_node(path))
				sfx[i] = cast_to<AudioStreamPlayer3D>(get_node(path));
			else if (i > 0)
				sfx[i] = sfx[i - 1];
		};
		// Collision
		for (int i = 0; i < get_child_count(); i++)
		{
			Node* c = get_child(i);
			if (c->get_class() == "CollisionShape")
			{
				col_node = cast_to<CollisionShape>(c);
				col_shape = cast_to<CollisionShape>(c)->get_shape();
				break;
			};
		};
		// Remove function set here for safety
		GAME->remove_check(this, spawnflags);
		if (current_state != ST_REMOVED)
		{
			// Target groups for trigger events
			add_to_group("ACTOR");
			if (properties.has("targetname"))
				GAME->set_node_targetname(this, properties["targetname"]);
			// Signal connections
			anim_player->connect("animation_finished", this, "_anim_finished");
			connect("enemy_found", this, "_enemy_found");
			GAME->connect("player_noise", this, "_heard_player");
			// Finalize
			call("col_set_solid");
			grav_set(get_global_transform());
			if (spawnflags & GameManager::FL_TELESPAWN)
			{
				hide();
				sfx_silence();
				set_collision_layer(0);
				set_collision_mask(0);
				emit_signal("collision_changed", 0);
				if (anim_player->has_animation("telespawn"))
					anim_player->play("telespawn");
			};
			set_think("start", 0.01f);
		};
	};
}

void Actor::_process(float delta)
{
	if (!Engine::get_singleton()->is_editor_hint())
	{
		if (has_node(enemy_path))
			enemy = cast_to<Spatial>(get_node(enemy_path));
		else
			enemy = nullptr;
		if (state_timer > 0.0f)
			state_timer -= delta;
		if (queue_timer > 0.0f)
			queue_timer -= delta;
		call("state_idle", delta);
		if (damaged > 0.0f)
			damaged -= delta;
		call("call_think");
	};
}

void Actor::_physics_process(float delta)
{
	if (!Engine::get_singleton()->is_editor_hint())
		call("state_physics",delta);
}

void Actor::_exit_tree()
{
	clear_enemy();
	emit_signal("actor_removed");
}
