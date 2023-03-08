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



// String to Vector3 function, because Godot has a hard time converting Vector3's back from JSON
// (the built in conversion function doesn't seem to work in GDNative)
Vector3 GameManager::str_to_vec3(String vec)
{
	vec = vec.replace("(", "");
	vec = vec.replace(")", "");
	vec = vec.replace(",", "");
	Array arr = vec.split(" ",false);
	Vector3 v = Vector3::ZERO;
	if (arr.size() == 3)
		for (int i = 0; i < 3; i++)
		{
			String s = arr[i];
			v[i] = s.to_float();
		};
	return v;
}

