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
#include "Player.h"

void Player::_register_methods()
{
	// Navigation
	register_method("nav_land", &Player::nav_land);
	register_method("is_crouching", &Player::is_crouching);
	register_method("enter_water", &Player::enter_water);
	register_method("exit_water", &Player::exit_water);
	// Combat
	register_method("wep_switch", &Player::wep_switch);
	register_method("_wep_switch", &Player::_wep_switch);
	register_property("wep_cooldown", &Player::set_wep_cooldown, &Player::get_wep_cooldown, 0.0f);
	register_property("wep_alt_cooldown", &Player::set_wep_alt_cooldown, &Player::get_wep_alt_cooldown, 0.0f);
	register_method("add_wep", &Player::add_wep);
	register_method("set_ammo", &Player::set_ammo);
	register_method("get_ammo", &Player::get_ammo);
	register_method("add_ammo", &Player::add_ammo);
	register_method("add_health", &Player::add_health);
	register_method("add_armor", &Player::add_armor);
	register_method("add_superdamage", &Player::add_superdamage);
	register_method("add_invincibility", &Player::add_invincibility);
	register_method("damage", &Player::damage);
	// Inventory
	register_method("add_item", &Player::add_item);
	register_method("get_items", &Player::get_items);
	register_method("save_items_to_start_status", &Player::save_items_to_start_status);
	// Sound
	register_method("snd_charge", &Player::snd_charge);
	register_method("snd_ductstep", &Player::snd_ductstep);
	// Scripting
	register_method("exit_map", &Player::exit_map);
	// Save Data
	register_method("data_save", &Player::data_save);
	register_method("data_load", &Player::data_load);
	// State Management
	register_method("state_enter", &Player::state_enter);
	register_method("state_idle", &Player::state_idle);
	register_method("state_physics", &Player::state_physics);
	// Base Processing
	register_method("_ready", &Player::_ready);
	// Signals
	register_signal<Player>("fire");
	register_signal<Player>("alt_fire");
	register_signal<Player>("unequip");
}

// Node Refs
Camera* Player::get_camera() { return camera; }

// Control
void Player::player_input()
{
	// Weapon Wheel
	if (CTRL->pressed("weapon_wheel"))
	{
		std::vector<int> a;
		for (int i = 0; i < WeaponManager::AMMO_TYPES; i++)
			a.push_back(ammo[i]);
		hud->ww_vis(weapons, a);
	};
	// Weapon wheel overrides actions
	if (CTRL->held("weapon_wheel"))
	{
		//move_input *= 0.0f;
		attack_input = 0;
		return;
	};
	// Movement
	move_input.z = float(CTRL->held("move_backward")) - float(CTRL->held("move_forward"));
	move_input.x = float(CTRL->held("strafe_right")) - float(CTRL->held("strafe_left"));
	// Speed management (nav_stance overrides this if crouching)
	if (CTRL->held("walk") || (CTRL->get_method() > 0 && CTRL->get_move_motion().length() < 0.7f))
		max_speed = walk_speed;
	else
		max_speed = run_speed;
	// Flying overrides jumping
	if (CTRL->held("jump") && flying)
		move_input.y = 1;
	// Jumping overrides crouch
	else if (CTRL->pressed("jump") && water_level < 2)
	{
		move_input.y = 1;
		jumping = true;
	}
	else if (CTRL->held("jump") && water_level >= 2)
		move_input.y = 1;
	// Crouch overrides not crouching
	else if (CTRL->held("crouch"))
		move_input.y = -1;
	else
		move_input.y = 0;
	// Interaction
	use_input = CTRL->pressed("use");
	// Combat
	if (CTRL->held("attack"))
		attack_input = 1;
	else if (CTRL->held("alt_attack"))
		attack_input = 2;
	else
		attack_input = 0;
	// Weapon swap
	for (int i = 1; i <= 10; i++)
		if (CTRL->pressed("weapon_" + String::num(i)))
		{
			wep_switch(i-1);
			break;
		};
	// Torch
	if (CTRL->pressed("torch"))
	{
		if (torch_power > 0.0f)
			torch_on = !torch_on;
		SND->play3d(sfx[CHAN_ITEM], s_torch);
	};
}

void Player::player_use()
{
	Vector2 s = get_viewport()->get_size() * 0.5f;
	for (int i = 0; i < 4; i++)
	{
		Dictionary col_dict = col_ray(camera->project_ray_origin(s), camera->project_position(s, 2.0f), GameManager::TRIGGER_LAYER + GameManager::MAP_LAYER, col_ex_self);
		if (col_dict.empty() == false)
		{
			Node* c = col_dict["collider"];
			if (i == 0 && c->is_in_group("WORLD"))
			{
				hud->set_use_visible(false);
				return;
			};
			if (c->is_in_group("TRIGGER_USE"))
			{
				if (use_input)
					c->call("trigger", this);
				if ((int)c->call("get_trigger_state") == 0)
					hud->set_use_visible(true);
				else
					hud->set_use_visible(false);
				return;
			};
			if (c->is_in_group("ITEM"))
			{
				if (use_input)
					c->call("pickup", this);
				hud->set_use_visible(true);
				return;
			};
		}
		else
			hud->set_use_visible(false);
	};
}

int Player::get_attack_input() { return attack_input; }

// Navigation
void Player::nav_rotate()
{
	if (CTRL->held("weapon_wheel"))
		return;
	Vector2 aim_input = CTRL->get_mouse_motion() * get_process_delta_time() * (float)GAME->target_fps * 0.001667f;
	if (CTRL->get_method() == ControlsManager::MODE::KEY)
		aim_input.y *= CTRL->get_mouse_invert_y();
	else
		aim_input.y *= CTRL->get_gamepad_invert_y();
	// Right / left rotation
	Transform t = get_transform();
	t.set_basis(t.basis.rotated(t.basis.y.normalized(), Math::deg2rad(-aim_input.x)));
	set_transform(t);
	// Up / down rotation
	aim_input.y = Math::clamp(aim_input.y + cam_x_rotation, -90.0f, 90.0f) - cam_x_rotation;
	cam_x_rotation += aim_input.y;
	camera->rotate_x(Math::deg2rad(-aim_input.y));
}

void Player::nav_set_direction(Basis basis_dir, Vector3 move_dir)
{
	Vector3 d = Vector3::ZERO;
	d = basis_dir.z * move_dir.z;
	d += basis_dir.x * move_dir.x;
	d += basis_dir.y * move_dir.y;
	nav_dir = d.normalized();
}

void Player::nav_land()
{
	float gv = grav_vector.length();
	if (gv > 1.0f)
	{
		if (gv < 30.0f)
			SND->play3d(sfx[CHAN_BODY], s_softland, 0, 0.5f);
		else
		{
			SND->play3d(sfx[CHAN_BODY], s_hardland, 10);
			camera->set_translation(camera->get_translation() + Vector3(0.0f, -0.13f, 0.0f));
		};
	};
}

void Player::nav_stance(float delta)
{
	if (current_state == ST_DEAD)
		return;
	float y = camera->get_translation().y, h = 0.65f;
	float t = fmaxf(0.625f * delta * GAME->ease(abs(y) / 0.625f, -2.0f), delta) * 3.0f;
	// Crouching
	if (move_input.y < 0 && water_level < 2 && on_floor)
	{
		max_speed = walk_speed;
		h = 0;
		if (y > h + t)
			camera->set_translation(Vector3(0.0f, fmaxf(y - t, h), 0.0f));
		else
		{
			crouching = true;
			col_stand->set_disabled(true);
			col_crouch->set_disabled(false);
		};
	}
	// Standing
	else
	{
		// We need to make sure we can stand first
		if (crouching)
		{
			max_speed = walk_speed;
			if (nav_check_move(-grav_dir * 0.65f) == false)
				return;
			crouching = false;
		}
		// It's safe to stand!
		else
		{
			col_stand->set_disabled(crouching);
			col_crouch->set_disabled(!crouching);
			if (y < h)
				camera->set_translation(Vector3(0.0f, fminf(y + t, h), 0.0f));
		};
	};
}

bool Player::is_crouching() { return crouching; }

// Water Navigation
void Player::enter_water(Area* new_water)
{
	Actor::enter_water(new_water);
	water_vol = new_water;
	if (water_level < 1)
	{
		if (GAME->get_time() > 0.1f)
			SND->play3d(sfx[CHAN_BODY], SND->S_WATER_ENTER, 50);
		water_level = 1;
	}
	if (new_water->get("liquid_color"))
		screen_shader->set_shader_param("water_tint", new_water->get("liquid_color"));
}

void Player::exit_water(Area* new_water)
{
	Actor::exit_water(new_water);
	if (new_water == water_vol)
		screen_shader->set_shader_param("underwater", false);
}

void Player::water_level_check()
{
	// Underwater
	if (water_3->get_overlapping_areas().size() > 0)
	{
		if (water_level < 2)
			grav_vector *= 0.0f;
		water_level = 3;
		screen_shader->set_shader_param("underwater", true);
	}
	// Swimming
	else if (water_2->get_overlapping_areas().size() > 0)
	{
		if (water_level < 2)
			grav_vector *= 0.0f;
		water_level = 2;
	}
	// Waist deep
	else
	{
		if (water_level >= 2)
			grav_vector *= 0.0f;
		water_level = 1;
	};
	if (water_level < 3)
		screen_shader->set_shader_param("underwater", false);
}


// Combat
void Player::wep_switch(int new_wep)
{
	if (have_wep(new_wep) == false || new_wep == wep_id || wep_cooldown > 0.0f || wep_alt_cooldown > 0.0f)
		return;
	wep_cooldown = 0.3f;
	if (wep_id >= 0)
	{
		wep_id = new_wep;
		emit_signal("unequip");
	}
	else
	{
		wep_id = new_wep;
		_wep_switch();
	};
	hud->ammo_type_update(WPN->WA_PAIR[wep_id]);
	hud->ammo_update(ammo[WPN->WA_PAIR[wep_id]], superdamage);
}

void Player::_wep_switch()
{
	for (int i = 0; i < camera->get_child_count(); i++)
	{
		Node* c = camera->get_child(i);
		if (c->is_in_group("V_WEP"))
		{
			c->set_name("remove");
			c->queue_free();
		};
	};
	if (wep_id >= 0)
	{
		Node* vw = WPN->get_v_wep(wep_id);
		vw->call("set_player", this);
		camera->call_deferred("add_child",vw);
	}
	else
		hud->ammo_type_update(-1);
}

void Player::set_wep_cooldown(float cool) { wep_cooldown = cool; }
float Player::get_wep_cooldown() { return wep_cooldown; }
void Player::set_wep_alt_cooldown(float cool) { wep_alt_cooldown = cool; }
float Player::get_wep_alt_cooldown() { return wep_alt_cooldown; }

bool Player::have_wep(int id)
{
	if (id < 0)
		return true;
	if (weapons & WPN->WEPS[id])
		return true;
	return false;
}

bool Player::add_wep(int id)
{
	if ((weapons & WPN->WEPS[id]) == 0)
	{
		weapons += WPN->WEPS[id];
		add_ammo(WPN->WA_PAIR[id], WPN->WA_START[id]);
		if (id > wep_id)
			wep_switch(id);
		item_flash(1.0f);
		return true;
	}
	else if (ammo[WPN->WA_PAIR[id]] < WPN->AMMO_MAX[WPN->WA_PAIR[id]])
		add_ammo(WPN->WA_PAIR[id], WPN->WA_START[id]);
		return true;
	return false;
}

void Player::set_ammo(int ammo_type, int amount)
{
	if (ammo_type < WeaponManager::AMMO_TYPES)
		ammo[ammo_type] = amount;
}

int Player::get_ammo(int ammo_type)
{
	if (ammo_type < WeaponManager::AMMO_TYPES)
		return ammo[ammo_type];
	else
		return 0;
}

bool Player::add_ammo(int ammo_type, int amount)
{
	if (ammo_type > -1)
		if (ammo[ammo_type] < WPN->AMMO_MAX[ammo_type])
		{
			ammo[ammo_type] = Math::min(ammo[ammo_type] + amount, WPN->AMMO_MAX[ammo_type]);
			item_flash();
			return true;
		};
	return false;
}

void Player::use_ammo(int ammo_type, int amount)
{
	if (ammo_type > -1 && GAME->get_infinite_ammo() == false)
		ammo[ammo_type] = Math::max(ammo[ammo_type] - amount, 0);
}

bool Player::add_health(int amount)
{
	if (amount >= health_max)
	{
		health = Math::min(health + amount, 200);
		hud->flash(Color(0.0f, 0.2f, 1.0f, 0.5f), 3.0f);
		return true;
	}
	else if (health < health_max)
	{
		health = Math::min(health + amount, health_max);
		item_flash();
		return true;
	};
	return false;
}

bool Player::add_armor(int armor_type)
{
	bool picked_up = false;
	if (armor_type == GameManager::IT_ARMORSHARD && armor < armor_max)
	{
		if (armor_max == 0)
		{
			items += GameManager::IT_ARMOR1;
			hud->armor_class_update(GameManager::IT_ARMOR1);
			armor_rating = 0.3f;
			armor_max = 100;
		};
		armor = Math::min(armor + 5, armor_max);
		picked_up = true;
	}
	else if (armor_type == GameManager::IT_ARMORPLATE && armor < armor_max)
	{
		if (armor_max == 0)
		{
			items += GameManager::IT_ARMOR1;
			hud->armor_class_update(GameManager::IT_ARMOR1);
			armor_rating = 0.3f;
			armor_max = 100;
		};
		armor = Math::min(armor + 25, armor_max);
		picked_up = true;
	}
	// Leather Armor
	else if (armor_type == GameManager::IT_ARMOR1 && armor < 100)
	{
		armor_rating = 0.3f;
		armor_max = 100;
		picked_up = true;
	}
	// Iron Armor
	else if (armor_type == GameManager::IT_ARMOR2 && armor < 150)
	{
		armor_rating = 0.5f;
		armor_max = 150;
		picked_up = true;
	}
	// Gold Armor
	else if (armor_type == GameManager::IT_ARMOR3 && armor < 200)
	{
		armor_rating = 0.777f;
		armor_max = 200;
		picked_up = true;
	};
	if (picked_up)
	{
		if (armor_type < GameManager::IT_ARMORSHARD)
		{
			armor = armor_max;
			items = items - (items & (GameManager::IT_ARMOR1 | GameManager::IT_ARMOR2 | GameManager::IT_ARMOR3)) + armor_type;
			hud->armor_class_update(armor_type);
			item_flash(1.0f);
		}
		else
			item_flash();
		hud->armor_update(armor, invincibility);
		return true;
	};
	return false;

}

void Player::add_superdamage()
{
	hud->flash(GameManager::get_color(GameManager::COL::CRIMSON, 0.5f), 3.0f);
	superdamage = 30.0f;
	screen_shader->set_shader_param("superdamage", true);
	SND->play3d(sfx[CHAN_ITEM], SND->S_SUPERDAMAGE[0], 10);
}

void Player::add_invincibility()
{
	hud->flash(GameManager::get_color(GameManager::COL::GOLD, 0.25f), 3.0f);
	invincibility = 30.0f;
	screen_shader->set_shader_param("invincibility", true);
	SND->play3d(sfx[CHAN_ITEM], SND->S_INVINCIBLITY[0], 10);
}

void Player::item_flash(float speed)
{
	if (hud->flash_rect->get_frame_color().a != 0.0f && speed < hud->get_flash_speed())
		speed = hud->get_flash_speed();
	hud->flash(Color(0.7f, 0.7f, 0.5f, 0.5f), speed);
}

void Player::damage(int amount, Node* attack, NodePath attacker)
{
	if (GAME->get_godmode())
		amount = 0;
	Actor::damage(amount, attack, attacker);
	if (health > 0 && sfx[CHAN_VOICE]->is_playing() == false)
	{
		if (invincibility > 0.0f)
			hud->flash(GameManager::get_color(GameManager::COL::GOLD, 0.25f), 1.0f);
		else
		{
			Node* a = get_node(attacker);
			int r = rng->randi() % 3;
			if ((attack != nullptr && attack->is_in_group("EXCURCIATING")) || (a != nullptr && a->is_in_group("EXCRUCIATING")))
				SND->play3d(sfx[CHAN_VOICE], s_pain_hi[r], 50);
			else if (health < health_max / 4)
				SND->play3d(sfx[CHAN_VOICE], s_pain_hi[r], 50);
			else if (health < health_max / 2)
				SND->play3d(sfx[CHAN_VOICE], s_pain_mid[r], 50);
			else
				SND->play3d(sfx[CHAN_VOICE], s_pain_lo[r], 50);
		};
	};
	if (invincibility <= 0.0f)
		hud->flash(Color(1.0f, 0.25f, 0.25f, 0.5f), 0.5f);
}

// Inventory
int Player::get_items() { return items; }
bool Player::has_item(int item_type) { return (items & item_type); }

bool Player::add_item(int item_type)
{
	if ((items & item_type) == false)
	{
		items += item_type;
		item_flash();
		return true;
	};
	return false;
}

bool Player::use_item(int item_type)
{
	if (items & item_type)
	{
		items -= item_type;
		return true;
	};
	return false;
}

void Player::save_items_to_start_status()
{
	int i = items;
	i &= ~(GameManager::IT_KEY1 | GameManager::IT_KEY2 | GameManager::IT_KEY3 | GameManager::IT_KEY4);
	Array status = Array::make(health, armor, armor_max, i, weapons, wep_id);
	for (int i = 0; i < WeaponManager::AMMO_TYPES; i++)
		status.append(ammo[i]);
	GAME->set_start_status(status);
}

// Audio
void Player::snd_charge()
{
	SND->play3d(sfx[CHAN_VOICE], s_charge);
}

void Player::snd_ductstep()
{
	if ((velocity - velocity * -grav_dir).length() > 0.01)
	{
		int r = rng->randi() % 7;
		SND->play3d(sfx[CHAN_BODY], ResourceLoader::get_singleton()->load("res://sounds/event/s_ductw" + String::num(r) + ".wav"));
	}
}

// SCRIPTING
void Player::exit_map()
{
	get_node("menu")->set("current_state", 666);
	camera->set_current(false);
	current_state = ST_REMOVED;
	think_check = false;
	hide();
	hud->hide();
	sfx_silence();
	set_collision_layer(0);
	set_collision_mask(0);
	set_translation(Vector3(-1000, -1000, -1000));
	emit_signal("actor_removed", get_path());
}

// SAVE DATA
Dictionary Player::data_save()
{
	Dictionary data = Actor::data_save();
	data["cam_x_rotation"] = cam_x_rotation;
	data["camera_rotation"] = camera->get_rotation();
	data["items"] = items;
	data["weapons"] = weapons;
	data["wep_id"] = wep_id;
	Array am = {};
	for (int i = 0; i < WeaponManager::AMMO_TYPES; i++)
		am.append(ammo[i]);
	data["ammo"] = am;
	data["torch_on"] = torch_on;
	data["torch_power"] = torch_power;
	return data;
}

void Player::data_load(Dictionary data)
{
	Actor::data_load(data);
	cam_x_rotation = data["cam_x_rotation"];
	camera->set_rotation(GameManager::str_to_vec3(data["camera_rotation"]));
	items = data["items"];
	weapons = data["weapons"];
	wep_id = data["wep_id"];
	Array am = data["ammo"];
	for (int i = 0; i < am.size(); i++)
		ammo[i] = am[i];
	torch_on = data["torch_on"];
	torch_power = data["torch_power"];
	hud->health_update(health);
	hud->armor_class_update(items & (GameManager::IT_ARMOR1 | GameManager::IT_ARMOR2 | GameManager::IT_ARMOR3));
	hud->armor_update(armor, invincibility);
	if (wep_id > WeaponManager::AX)
		hud->ammo_update(ammo[WPN->WA_PAIR[wep_id]], superdamage);
	hud->ammo_type_update(WPN->WA_PAIR[wep_id]);
	hud->pitch_update(cam_x_rotation);
	hud->torch_update(get_process_delta_time(), torch_on, torch_power);
	wep_switch(wep_id);
}

// STATE MANAGEMENT
void Player::state_enter()
{
	Actor::state_enter();
	if (current_state == ST_DEAD)
	{
		CTRL->release_all();
		hud->flash(Color(1.0f, 0.25f, 0.25f, 1.0f), 3.0f);
		move_input *= 0.0f;
		wep_id = -1;
		hud->ammo_type_update(-1);
		emit_signal("unequip");
		anim_player->play("die");
		sfx_set_vol(sfx[CHAN_VOICE], 1.0f);
		state_timer = 2.0f;
	};
}

void Player::state_idle(float delta)
{
	Actor::state_idle(delta);
	// Health rot
	if (health > health_max)
	{
		if (health_rot_ct > 0.0f)
			health_rot_ct -= delta;
		else
		{
			health = Math::max(health - 1, health_max);
			health_rot_ct = 1.0f;
		};
	};
	// Powerup rot
	Color powerup_color = Color();
	powerup_light->hide();
	if (superdamage > 0.0f)
	{
		superdamage -= delta;
		if (superdamage <= 5.0f && int(superdamage * 10.0f) % 10 == 0)
		{
			hud->flash(GameManager::get_color(GameManager::COL::CRIMSON, 0.5f), 3.0f);
			SND->play3d(sfx[CHAN_ITEM], SND->S_SUPERDAMAGE[1], 2, Math::lerp(0.1f, 1.0f, superdamage / 10.0f));
		};
		powerup_color = GameManager::get_color(GameManager::COL::CRIMSON);
		powerup_light->show();
		if (superdamage <= 0.0f)
			screen_shader->set_shader_param("superdamage", false);
	};
	if (invincibility > 0.0f)
	{
		invincibility -= delta;
		if (invincibility <= 5.0f && int(invincibility * 10.0f) % 10 == 0)
		{
			hud->flash(GameManager::get_color(GameManager::COL::GOLD, 0.5f), 3.0f);
			SND->play3d(sfx[CHAN_ITEM], SND->S_INVINCIBLITY[1], 1, Math::lerp(0.1f, 1.0f, invincibility / 10.0f));
		};
		if (superdamage > 0.0f)
			powerup_color = powerup_color.linear_interpolate(GameManager::get_color(GameManager::COL::GOLD), 0.5f);
		else
			powerup_color = GameManager::get_color(GameManager::COL::GOLD);
		powerup_light->show();
		if (invincibility <= 0.0f)
			screen_shader->set_shader_param("invincibility", false);
	};
	powerup_light->set_color(powerup_color);
	wep_view->set_global_transform(camera->get_global_transform());
	// State logic
	switch (current_state)
	{
	case ST_IDLE:
		player_input();
		//nav_rotate(delta);
		player_use();
		// Animation
		if (on_floor)
		{
			if (round(velocity.length() * 2.5f) > 0.0f)
			{
				if (crouching)
					anim_player->play("c_walk");
				else if (max_speed == walk_speed)
					anim_player->play("walk");
				else
					anim_player->play("run");
			}
			else
			{
				if (crouching)
					anim_player->play("c_idle");
				else
					anim_player->play("idle");
			};
		}
		else
		{
			if (water_level >= 2)
				anim_player->play("swim");
			else
				anim_player->play("fall");
		};
		// Attack input
		if (wep_id >= 0)
		{
			if (wep_cooldown > 0.0f)
				wep_cooldown -= delta;
			else if (attack_input == 1)
			{
				emit_signal("fire");
				GAME->emit_signal("player_noise", get_global_translation());
			};
			if (wep_alt_cooldown > 0.0f)
				wep_alt_cooldown -= delta;
			else if (attack_input == 2)
			{
				emit_signal("alt_fire");
				GAME->emit_signal("player_noise", get_global_translation());
			};
		};
		chase_add_breadcrumb(4.0f);
		if (CTRL->pressed("quicksave") && health > 0)
		{
			if (get_node("/root/SaveManager")->call("save_game", -1))
				SND->menu_open();
			else
				SND->menu_error();
		};
		break;
	case ST_DEAD:
		//nav_rotate(delta);
		float ez = GAME->ease(state_timer / 2.0f, -2.0f);
		if (gibbed && sfx[CHAN_VOICE]->is_playing())
			sfx[CHAN_VOICE]->stop();
		if (camera->get_translation().y > -0.75f)
			camera->set_translation(camera->get_translation() - Vector3(0.0f, 0.75f * delta * ez * 3.0f, 0.0f));
		if (camera->get_rotation().z > -0.785398f)
			camera->set_rotation(camera->get_rotation() - Vector3(0.0f, 0.0f, 1.570796f * delta * ez));
		if (state_timer <= 0.0f)
		{
			if (CTRL->pressed("use") || CTRL->pressed("jump") || CTRL->pressed("attack"))
				GAME->change_map(GAME->current_map.id);
			else if (!CTRL->pressed("menu"))
				CTRL->release_all();
		};
		break;
	};
	// Torch
	if (torch_on)
	{
		if (GAME->get_infinite_torch() == false)
			torch_power -= delta * 0.0333f;
		torch->show();
		torch->set(SpotLight::PARAM_ENERGY, Math::lerp(0.0f, 1.0f, GAME->ease(torch_power, 0.1f)));
		wep_light->set(OmniLight::PARAM_ENERGY, torch->get(SpotLight::PARAM_ENERGY));
		if (torch_power < 0.0f)
		{
			torch_power = -0.13f;
			torch_on = false;
			SND->play3d(sfx[CHAN_ITEM], s_torchdie);
		};
	}
	else
	{
		if (torch_power < 1.0f)
			torch_power = fminf(torch_power + delta * 0.05f, 1.0f);
		torch->hide();
	};
	hud->health_update(health);
	hud->armor_update(armor, invincibility);
	hud->ammo_update(ammo[WPN->WA_PAIR[wep_id]], superdamage);
	hud->pitch_update(cam_x_rotation);
	hud->torch_update(delta, torch_on, torch_power);
	if (CTRL->pressed("quickload") && !CTRL->pressed("quicksave"))
	{
		if (get_node("/root/SaveManager")->call("load_game", -1))
			SND->menu_close();
		else
			SND->menu_error();
	};
}

void Player::state_physics(float delta)
{
	Actor::state_physics(delta);
	if (water_level > 0)
		water_level_check();
	switch (current_state)
	{
	case ST_IDLE:
	case ST_DEAD:
		nav_rotate();
		nav_stance(delta);
		if (water_level < 2 && flying == false)
		{
			nav_set_direction(get_global_transform().basis, move_input);
			nav_grav_accel(delta);
			if (jumping && on_floor && water_level < 3)
				SND->play3d(sfx[CHAN_VOICE], s_jump[rng->randi() % 3]);
			nav_move(delta);
		}
		else
		{
			if (flying && max_speed == run_speed)
				max_speed = 25.0;
			//if (move_input.y > 0 && water_level == 2)
			//	velocity = check_water_jump(velocity);
			nav_set_direction(camera->get_global_transform().basis, move_input);
			nav_fly_move(delta);
		};
		if (get_collision_mask() > 0)
			nav_grav_dir();
		nav_xform(delta);
	};
}

// BASE PROCESSING
void Player::_init()
{
	in_pvs = true;
	spawnflags = 0;
	classname = "player";
	actorflags = GameManager::FL_PLAYER;
	Actor::_init();
	// Combat
	gib_threshold = -40;
	// Preloads
	ResourceLoader* loader = ResourceLoader::get_singleton();
	s_torch = loader->load("res://sounds/actor/player/s_player_torch.wav");
	s_torchdie = loader->load("res://sounds/actor/player/s_player_torchdie.wav");
	s_softland = loader->load("res://sounds/actor/player/s_player_softland0.wav");
	s_hardland = loader->load("res://sounds/actor/player/s_player_hardland.wav");
	for (int i = 0; i < 3; i++)
	{
		s_jump[i] = loader->load("res://sounds/actor/player/s_player_jump" + String::num(i) + ".wav");
		s_pain_lo[i] = loader->load("res://sounds/actor/player/s_player_pain_lo" + String::num(i) + ".wav");
		s_pain_mid[i] = loader->load("res://sounds/actor/player/s_player_pain_mid" + String::num(i) + ".wav");
		s_pain_hi[i] = loader->load("res://sounds/actor/player/s_player_pain_hi" + String::num(i) + ".wav");
	};
	s_die.push_back(loader->load("res://sounds/actor/player/s_player_die.wav"));
	s_charge = loader->load("res://sounds/actor/player/s_player_axe_big.wav");
	gib_res.push_back(loader->load("res://entities/actors/player/gib_player.tscn"));
	gib_res.push_back(loader->load("res://entities/actors/explorer/gibs/gib_explorer1.tscn"));
	gib_res.push_back(loader->load("res://entities/actors/explorer/gibs/gib_explorer2.tscn"));
	gib_res.push_back(loader->load("res://entities/actors/explorer/gibs/gib_explorer3.tscn"));
}

void Player::_ready()
{
	if (!Engine::get_singleton()->is_editor_hint())
	{
		CTRL = cast_to<ControlsManager>(get_node("/root/ControlsManager"));
		CTRL->mouse_lock(true);
		get_tree()->set_pause(false);
		WPN = cast_to<WeaponManager>(get_node("/root/WeaponManager"));
		GAME->set_player_node(this);
		// Set starting status and inventory
		Array status;
		if (GAME->get_game_mode() <= GameManager::COOP)
			status = GAME->get_start_status();
		else
			status = Array::make(100, 100, 100, 0, WeaponManager::IT_AX + WeaponManager::IT_XP, 2, 25);
		health = status[GameManager::HEALTH];
		armor = status[GameManager::ARMOR];
		armor_max = status[GameManager::ARMOR_MAX];
		items = status[GameManager::ITEMS];
		weapons = status[GameManager::WEAPONS];
		wep_id = status[GameManager::START_WEP];
		for (int i = 0; i <= WeaponManager::AMMO_TYPES; i++)
		{
			if (i < status.size())
				ammo[i] = status[i + GameManager::START_WEP + 1];
			else
				ammo[i] = 0;
		};
		// Onready
		camera = cast_to<Camera>(get_node("cam"));
		water_2 = cast_to<Area>(get_node("water_2"));
		water_3 = cast_to<Area>(get_node("water_3"));
		col_stand = cast_to<CollisionShape>(get_node("c"));
		col_crouch = cast_to<CollisionShape>(get_node("c_lo"));
		torch = cast_to<SpotLight>(get_node("cam/torch"));
		wep_light = cast_to<OmniLight>(get_node("cam/torch/wep_light"));
		wep_view = cast_to<Camera>(get_node("cam/vc_wep/vp_wep/wep_view"));
		hud = cast_to<Hud>(get_node("hud"));
		screen_shader = get_node("screen_shader")->get("material");
		powerup_light = cast_to<OmniLight>(get_node("pwr_light"));
		for (int i = 0; i < 4; i++)
		{
			NodePath path = NodePath("cam/sfx" + String::num(i));
			if (has_node(path))
				sfx[i] = cast_to<AudioStreamPlayer3D>(get_node(path));
		};
		// Signal connections
		GAME->connect("fov_updated", camera, "set_fov");
		hud->connect("wep_wheel_pick", this, "wep_switch");
		connect("just_landed", this, "nav_land");
		GAME->disconnect("player_noise", this, "_heard_player");
		disconnect("enemy_found", this, "_enemy_found");
		// View defaults
		camera->set_fov(GAME->get_fov());
		screen_shader->set_shader_param("superdamage", false);
		screen_shader->set_shader_param("invincibility", false);
		wep_view->set_environment(camera->get_environment());
		hud->health_update(health);
		hud->armor_class_update(items & (GameManager::IT_ARMOR1 | GameManager::IT_ARMOR2 | GameManager::IT_ARMOR3));
		hud->armor_update(armor,invincibility);
		if (wep_id > WeaponManager::AX)
			hud->ammo_update(ammo[WPN->WA_PAIR[wep_id]], superdamage);
		hud->ammo_type_update(WPN->WA_PAIR[wep_id]);
		hud->pitch_update(cam_x_rotation);
		hud->torch_update(get_process_delta_time(), torch_on, torch_power);
		// Final prep
		call_deferred("_wep_switch");
	};
}
