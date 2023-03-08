/***************************************************
MUSIC MANAGER CLASS
****************************************************/
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Godot.hpp"
#include <Math.hpp>
#include <ResourceLoader.hpp>
#include <PackedScene.hpp>
#include <Directory.hpp>
#include <File.hpp>
#include <JSONParseResult.hpp>
#include "JSON.hpp"
#include "AudioServer.hpp"
#include "AudioStreamPlayer.hpp"

class MusicManager : public AudioStreamPlayer
{
private:
	GODOT_CLASS(MusicManager, AudioStreamPlayer);
	struct song_struct { String id; String name; String file; std::vector<std::vector<float>> loops; };
	std::unordered_map<std::string, song_struct> song_defs = {};
	song_struct current_song;
	int current_loop_index = 0, last_loop_index = 0;
	float current_loop[2] = { 0.0f }, volume_target = 0.0f, volume_delta = 0.0f;
	Array saved_loop;
	bool keep_looping = true, pausing = false, resuming = true;
	void build_song_defs();
	void load_song(std::string song_id);
public:
	static void _register_methods();
	void play_loop(int loop_id);
	void pause_loop();
	void resume_loop();
	void exit_loop();
	String get_song_name(String song_id);
	int get_current_loop();
	bool is_song_playing(String song_id);
	bool is_loop_playing(int loop_id);
	void change_volume(float target = 1.0f, float delay = 0.0f);
	void music_play(String song_id, int loop_id = 0, float vol_target = 1.0f, float delay = 0.0f);
	void music_pause(float delay = 0.5f);
	void music_resume(float vol_target = 1.0f, float delay = 1.0f);
	Dictionary data_save();
	void data_load(Dictionary data);
	void _init();
	void _process(float delta);
};
