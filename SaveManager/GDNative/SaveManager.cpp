/*******************************************************************************
SAVE MANAGER CLASS
Handles both config saving and save games.
*******************************************************************************/
#include "SaveManager.h"

void SaveManager::_register_methods()
{
	register_method("save_game", &SaveManager::save_game);
	register_method("load_game", &SaveManager::load_game);
	register_method("_load_game", &SaveManager::_load_game);
	register_method("_load_player", &SaveManager::_load_player);
	register_method("_ready", &SaveManager::_ready);
	register_signal<SaveManager>("load_complete");
}

void SaveManager::save_config()
{
	ControlsManager* CTRL = cast_to<ControlsManager>(get_node("/root/ControlsManager"));
	SoundManager* SND = cast_to<SoundManager>(get_node("/root/SoundManager"));
	Ref<ConfigFile> cfg = Ref<ConfigFile>(ConfigFile::_new());
	// Version
	cfg->set_value("Version", "config_version", CONFIG_VERSION);
	// Sound
	cfg->set_value("Sound", "music_volume", SND->get_bus_vol("Music"));
	cfg->set_value("Sound", "sfx_volume", SND->get_bus_vol("Sfx"));
	// Display
	cfg->set_value("Display", "window_size", OS::get_singleton()->get_window_size());
	cfg->set_value("Display", "fullscreen", OS::get_singleton()->is_window_fullscreen());
	cfg->set_value("Display", "borderless", OS::get_singleton()->get_borderless_window());
	cfg->set_value("Display", "fov", GAME->get_fov());
	cfg->set_value("Display", "brightness", GAME->get_brightness());
	cfg->set_value("Display", "fps", Engine::get_singleton()->get_target_fps());
	cfg->set_value("Display", "hud_visible", GAME->get_hud_vis());
	cfg->set_value("Display", "weapon_visible", GAME->get_wep_vis());
	// Controls
	cfg->set_value("Controls", "mouse_sensitivity", CTRL->get_mouse_sensitivity());
	cfg->set_value("Controls", "mouse_invert_y", CTRL->get_mouse_invert_y());
	cfg->set_value("Controls", "gamepad_invert_y", CTRL->get_gamepad_invert_y());
	cfg->set_value("Controls", "key_map", CTRL->get_map_dict(ControlsManager::MODE::KEY));
	cfg->set_value("Controls", "xbox_map", CTRL->get_map_dict(ControlsManager::MODE::XBOX));
	cfg->set_value("Controls", "ps4_map", CTRL->get_map_dict(ControlsManager::MODE::PS4));
	cfg->set_value("Controls", "snes_map", CTRL->get_map_dict(ControlsManager::MODE::SNES));
	// Save it!
	cfg->save("user://tcfdx.cfg");
}

void SaveManager::load_config()
{
	Ref<ConfigFile> cfg = Ref<ConfigFile>(ConfigFile::_new());
	Error err = cfg->load("user://tcfdx.cfg");
	// Check if the config file exists. If not, make one.
	if (err != Error::OK)
	{
		save_config();
		return;
	};
	// Check if the config version matches. If not, remake it.
	// That way we don't accidentally end up with bad values on config updates.
	if ((int)cfg->get_value("Version", "config_version") != CONFIG_VERSION)
	{
		save_config();
		return;
	};
	// Time to load the config!
	ControlsManager* CTRL = cast_to<ControlsManager>(get_node("/root/ControlsManager"));
	SoundManager* SND = cast_to<SoundManager>(get_node("/root/SoundManager"));
	OS* os = OS::get_singleton();
	// Sound
	SND->set_bus_vol("Music", cfg->get_value("Sound", "music_volume", 1.0f), 0.0f);
	SND->set_bus_vol("Sfx", cfg->get_value("Sound", "sfx_volume", 1.0f), 0.0f);
	// Display
	if ((bool)cfg->get_value("Display", "fullscreen", true) == true)
	{
		os->set_window_size(os->get_screen_size());
		os->set_window_fullscreen(true);
		os->set_window_resizable(false);
	}
	else
	{
		os->set_window_fullscreen(false);
		os->set_window_resizable(true);
		os->set_window_size(cfg->get_value("Display", "window_size", Vector2(1280, 720)));
		os->set_window_position(os->get_screen_size() * 0.5f - os->get_window_size() * Vector2(0.5f, 0.55f));
	};
	if ((bool)cfg->get_value("Display", "borderless", false) == true)
		os->set_borderless_window(true);
	GAME->set_fov(cfg->get_value("Display", "fov", 90.0f));
	GAME->set_brightness(cfg->get_value("Display", "brightness", 1.0f));
	GAME->target_fps = cfg->get_value("Display", "fps", 60);
	Engine::get_singleton()->set_target_fps(GAME->target_fps);
	GAME->set_hud_vis(cfg->get_value("Display", "hud_visible", GAME->get_hud_vis()));
	GAME->set_wep_vis(cfg->get_value("Display", "weapon_visible", GAME->get_wep_vis()));
	// Controls
	CTRL->set_dict_to_map(ControlsManager::MODE::KEY, (Dictionary)cfg->get_value("Controls", "key_map", CTRL->get_map_dict(ControlsManager::MODE::KEY)));
	CTRL->set_dict_to_map(ControlsManager::MODE::XBOX, (Dictionary)cfg->get_value("Controls", "xbox_map", CTRL->get_map_dict(ControlsManager::MODE::XBOX)));
	CTRL->set_dict_to_map(ControlsManager::MODE::PS4, (Dictionary)cfg->get_value("Controls", "ps4_map", CTRL->get_map_dict(ControlsManager::MODE::PS4)));
	CTRL->set_dict_to_map(ControlsManager::MODE::SNES, (Dictionary)cfg->get_value("Controls", "snes_map", CTRL->get_map_dict(ControlsManager::MODE::SNES)));
	CTRL->set_control_map(CTRL->get_method());
	CTRL->set_mouse_sensitivity(cfg->get_value("Controls", "mouse_sensitivity", CTRL->get_mouse_sensitivity()));
	CTRL->set_mouse_invert_y(cfg->get_value("Controls", "mouse_invert_y", CTRL->get_mouse_invert_y()));
	CTRL->set_gamepad_invert_y(cfg->get_value("Controls", "gamepad_invert_y", CTRL->get_gamepad_invert_y()));
}

bool SaveManager::save_game(int data_id)
{
	if (GAME->get_game_mode() != GameManager::SINGLEPLAYER)
		return false;
	Dictionary data;
	Ref<File> file = Ref<File>(File::_new());
	Error file_chk;
	if (data_id >= 0)
		file_chk = file->open("user://saves/" + String::num(data_id) + ".sav", File::WRITE);
	else
		file_chk = file->open("user://saves/quick.sav", File::WRITE);
	if (file_chk != Error::OK)
	{
		String msg = (data_id >= 0) ? "Unable to save game!" : "Unable to quicksave!";
		GAME->trigger_notification(msg);
		return false;
	};
	data["save_version"] = SAVE_VERSION;
	data["save_id"] = data_id;
	data["start_status"] = GAME->get_start_status();
	data["map"] = GAME->current_map.id;
	data["mapname"] = GAME->current_map.name;
	data["time"] = GAME->get_time();
	Dictionary ent_data = {};
	Array ents = get_tree()->get_nodes_in_group("SAV");
	for (int i = 0; i < ents.size(); i++)
	{
		Node* e = ents[i];
		if (e->has_method("data_save"))
			ent_data[String(e->get_path())] = e->call("data_save");
	};
	data["entities"] = ent_data;
	file->store_line(data.to_json());
	String msg = (data_id >= 0) ? "Game saved" : "Game quicksaved";
	GAME->trigger_notification(msg);
	file->close();
	return true;
}

bool SaveManager::load_game(int data_id)
{
	if (GAME->get_game_mode() != GameManager::SINGLEPLAYER)
		return false;
	Ref<File> file = Ref<File>(File::_new());
	Error file_chk;
	if (data_id >= 0)
		file_chk = file->open("user://saves/" + String::num(data_id) + ".sav", File::READ);
	else
		file_chk = file->open("user://saves/quick.sav", File::READ);
	if (file_chk == Error::OK)
	{
		Dictionary data = (Dictionary)JSON::get_singleton()->parse(file->get_as_text())->get_result();
		// Need a handler to omit any saves that don't match the current version
		if ((int)data["save_version"] != SAVE_VERSION)
		{
			GAME->trigger_notification("Incorrect save version!");
			file->close();
			return false;
		};
		String msg = (data_id >= 0) ? "Loading save..." : "Loading quicksave...";
		GAME->trigger_notification(msg);
		load_cache = data;
		GAME->set_start_status(data["start_status"]);
		GAME->change_map(data["map"]);
		file->close();
		return true;
	};
	String msg = (data_id >= 0) ? "Unable to load save!" : "Unable to load quicksave!";
	GAME->trigger_notification(msg);
	return false;
}

void SaveManager::_load_game()
{
	if (load_cache.empty())
		return;
	GAME->set_time(load_cache["time"]);
	Dictionary ent_data = load_cache["entities"];
	Array keys = ent_data.keys();
	for (int i = 0; i < keys.size(); i++)
	{
		NodePath np = keys[i];
		if (has_node(np))
		{
			Node* ent = get_node(np);
			if (ent->has_method("data_load"))
				ent->call("data_load", ent_data[keys[i]]);
		}
		else
		{
			String s = keys[i];
			Dictionary d = ent_data[keys[i]];
			if (d.has("filename"))
			{
				Node* ent = Ref<PackedScene>(ResourceLoader::get_singleton()->load(d["filename"]))->instance();
				get_tree()->get_current_scene()->add_child(ent);
				if (ent->has_method("data_load"))
					ent->call_deferred("data_load", d);
			}
			else if (s.find("player") >= 0)
				player_cache[keys[i]] = ent_data[keys[i]];
		};
	};
	load_cache.clear();
}

void SaveManager::_load_player()
{
	if (player_cache.empty())
		return;
	Dictionary player_data = player_cache;
	Array keys = player_data.keys();
	for (int i = 0; i < keys.size(); i++)
	{
		NodePath np = keys[i];
		if (has_node(np))
		{
			Node* p = get_node(np);
			if (p->has_method("data_load"))
				p->call("data_load", player_data[keys[i]]);
		};
	};
	player_cache.clear();
}

Array SaveManager::get_save_list(bool empty_slots)
{
	Array save_list = {};
	Ref<Directory> dir = Ref<Directory>(Directory::_new());
	if (dir->open("user://saves") == Error::OK)
	{
		dir->list_dir_begin();
		String filename = dir->get_next();
		int i = 10;
		while (filename != "" && i > 0)
		{
			if (filename.rfind(".sav") > -1)
			{
				Ref<File> file = Ref<File>(File::_new());
				if (file->open(dir->get_current_dir() + "/" + filename, File::READ) == Error::OK)
				{
					Dictionary data = (Dictionary)JSON::get_singleton()->parse(file->get_as_text())->get_result();
					String s = (filename == "quick.sav") ? "Quicksave - " : "";
					String m = data["mapname"];
					if (m.length() > 20)
						m = m.left(20) + "...";
					s += m + " - " + GameManager::get_time_string(data["time"]);
					save_list.append(s);
					if (empty_slots && filename == "quick.sav")
						save_list.remove(save_list.size() - 1);
					i--;
					file->close();
				};
			};
			filename = dir->get_next();
		};
		if (empty_slots && i > 1)
			save_list.append(String("--- Unused Slot ---"));
	};
	return save_list;
}

void SaveManager::_init()
{
	load_cache.clear();
	player_cache.clear();
}

void SaveManager::_ready()
{
	CTRL = cast_to<ControlsManager>(get_node("/root/ControlsManager"));
	GAME = cast_to<GameManager>(get_node("/root/GameManager"));
	GAME->connect("map_ready", this, "_load_game");
	GAME->connect("player_spawned", this, "_load_player");
	MUSIC = cast_to<MusicManager>(get_node("/root/MusicManager"));
	Ref<Directory> dir = Ref<Directory>(Directory::_new());
	if (dir->open("user://saves") != Error::OK) { dir->make_dir("user://saves"); };
	load_config();
}
