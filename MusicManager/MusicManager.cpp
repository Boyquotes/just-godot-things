/***************************************************
MUSIC MANAGER CLASS
****************************************************/
#include "MusicManager.h"

void MusicManager::_register_methods()
{
	register_method("music_play", &MusicManager::music_play);
	register_method("music_pause", &MusicManager::music_pause);
	register_method("music_resume", &MusicManager::music_resume);
	register_method("change_volume", &MusicManager::change_volume);
	register_method("data_save", &MusicManager::data_save);
	register_method("data_load", &MusicManager::data_load);
	register_method("_process", &MusicManager::_process);
}

void MusicManager::build_song_defs()
{
	for (int d = 0; d < 2; d++)
	{
		Ref<Directory> dir = Ref<Directory>(Directory::_new());
		Error dir_chk;
		if (d == 0)
			dir_chk = dir->open("res://music");
		else
			dir_chk = dir->open("user://music");
		if (dir_chk == Error::OK)
		{
			dir->list_dir_begin();
			String filename = dir->get_next();
			while (filename != "")
			{
				if (filename.rfind(".json") > -1)
				{
					Ref<File> file = Ref<File>(File::_new());
					if (file->open(dir->get_current_dir() + "/" + filename, File::READ) == Error::OK)
					{
						Array song_data = (Array)JSON::get_singleton()->parse(file->get_as_text())->get_result();
						for (int i = 0; i < song_data.size(); i++)
						{
							Dictionary data = song_data[i];
							song_struct song;
							song.id = data["id"];
							song.name = data["name"];
							song.file = data["file"];
							Array loop_arr = data["loops"];
							for (int j = 0; j < loop_arr.size(); j++)
							{
								Array l2 = loop_arr[j];
								song.loops.push_back({ (float)l2[0],(float)l2[1] });
							};
							song_defs[song.id.alloc_c_string()] = song;
						};
						file->close();
					};
				};
				filename = dir->get_next();
			};
		}
		else if (d > 0)
			dir->make_dir("user://music");
	};
}

void MusicManager::load_song(std::string song_id)
{
	if (song_defs.find(song_id) == song_defs.end())
	{
		stop();
		return;
	};
	current_song = song_defs[song_id];
	Ref<AudioStream> s = ResourceLoader::get_singleton()->load(current_song.file);
	set_stream(s);
	last_loop_index = 0;
	seek(0.0f);
}

void MusicManager::play_loop(int loop_id)
{
	if (loop_id > -1 && current_song.loops.size() > loop_id)
	{
		// Make sure we aren't already past the desired loop
		if (get_playback_position() > current_loop[1])
			return;
		// Loop setup
		last_loop_index = current_loop_index;
		current_loop_index = loop_id;
		current_loop[0] = current_song.loops[loop_id][0];
		current_loop[1] = current_song.loops[loop_id][1];
		if (current_loop[1] <= current_loop[0])
			keep_looping = false;
	}
	// No loops available
	else
	{
		last_loop_index = current_loop_index;
		current_loop_index = 0;
		keep_looping = false;
	};
	// Play it again, Sam
	set_stream_paused(false);
	if (!is_playing())
		play();
}

// Pause / Resume; when we pause, we save the current position so that we can pick up where we left off
// Main use case is switching to a temporary scene (a quick special menu or event) and then restoring the main music after returning to the persistent scene
// i.e: in an RPG, entering a battle yielding battle music, returning to the area map restoring the area map music from where it left off
void MusicManager::pause_loop()
{
	if (!get_stream_paused())
	{
		saved_loop = Array::make(current_song.id, current_loop_index, get_playback_position());
		set_stream_paused(true);
	};
}

void MusicManager::resume_loop()
{
	if (get_stream_paused() || !is_playing())
	{
		if (saved_loop.size() < 3)
			saved_loop = Array::make(current_song.id, current_loop_index, get_playback_position());
		String id = saved_loop[0];
		load_song(id.alloc_c_string());
		play_loop(saved_loop[1]);
		seek(saved_loop[2]);
	};
}

// Allow the song to finish
void MusicManager::exit_loop() { keep_looping = false; }

String MusicManager::get_song_name(String song_id) { return current_song.name; }

int MusicManager::get_current_loop() { return current_loop_index; }

bool MusicManager::is_song_playing(String song_id)
{
	if (is_playing() && song_id == current_song.id)
		return true;
	return false;
}

bool MusicManager::is_loop_playing(int loop_id)
{
	if (is_playing() && current_loop_index == loop_id)
		return true;
	return false;
}

// Volume adjustment; instantly change if time is negative;
void MusicManager::change_volume(float target, float delay)
{
	volume_target = fmaxf(0.0f, target);
	if (delay > 0.0f)
		volume_delta = 1.0f / delay;
	else
		set_volume_db(Math::linear2db(target));
}

void MusicManager::music_play(String song_id, int loop_id, float vol_target, float delay)
{
	std::string id = song_id.alloc_c_string();
	if (song_defs.find(id) == song_defs.end())
		music_pause(delay);
	else
	{
		load_song(id);
		play_loop(loop_id);
		change_volume(vol_target, delay);
	};
}

void MusicManager::music_pause(float delay)
{
	change_volume(0.0f, delay);
	pausing = true;
}

void MusicManager::music_resume(float vol_target, float delay)
{
	resume_loop();
	change_volume(vol_target, delay);
}

Dictionary MusicManager::data_save()
{
	Dictionary data;
	data["playing"] = is_playing();
	data["pausing"] = pausing;
	data["paused"] = get_stream_paused();
	data["song"] = current_song.id;
	data["loop"] = current_loop_index;
	data["keep_looping"] = keep_looping;
	data["position"] = get_playback_position();
	data["volume"] = Math::db2linear(get_volume_db());
	data["volume_target"] = volume_target;
	data["volume_delta"] = volume_delta;
	data["saved_loop"] = saved_loop;
	return data;
}

void MusicManager::data_load(Dictionary data)
{
	saved_loop = data["saved_loop"];
	music_play(data["song"], data["loop"], data["volume_target"], 0.0f);
	volume_delta = data["volume_delta"];
	keep_looping = data["keep_looping"];
	seek(data["position"]);
	float v = data["volume"];
	set_volume_db(Math::linear2db(v));
	pausing = data["pausing"];
	set_stream_paused(data["paused"]);
	if ((bool)data["playing"] == false)
		stop();
}

void MusicManager::_init()
{
	set_pause_mode(PAUSE_MODE_PROCESS);
	if (AudioServer::get_singleton()->get_bus_index("Music") > -1)
		set_bus("Music");
	else
		set_bus("Master");
	build_song_defs();
	add_to_group("SAV");
}

void MusicManager::_process(float delta)
{
	// Volume change
	float v = Math::db2linear(get_volume_db());
	if (v != volume_target)
	{
		if (v > volume_target)
			v = fmaxf(v - volume_delta * delta, volume_target);
		else if (v < volume_target)
			v = fminf(v + volume_delta * delta, volume_target);
		set_volume_db(Math::linear2db(v));
		// Pausing
		if (pausing && v <= 0.0f)
		{
			pausing = false;
			pause_loop();
		};
	};
	// Looping
	if (keep_looping && get_playback_position() >= current_loop[1])
		seek(current_loop[0]);
}
