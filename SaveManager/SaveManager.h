/*******************************************************************************
SAVE MANAGER CLASS
Handles both config saving and save games.
*******************************************************************************/
#pragma once
#include "Common.h"
#include <JSONParseResult.hpp>
#include "JSON.hpp"
#include <Directory.hpp>
#include <File.hpp>
#include <ConfigFile.hpp>
#include "OS.hpp"
#include "ControlsManager.h"
#include "SoundManager.h"
#include "GameManager.h"

class SaveManager : public Node
{
private:
	GODOT_CLASS(SaveManager, Node);
	const int CONFIG_VERSION = 2, SAVE_VERSION = 1;
	ControlsManager* CTRL; GameManager* GAME; MusicManager* MUSIC;
	Dictionary load_cache = {}, player_cache = {};
public:
	static void _register_methods();
	void save_config();
	void load_config();
	bool save_game(int data_id = -1);
	bool load_game(int data_id = -1);
	void _load_game();
	void _load_player();
	Array get_save_list(bool empty_slots = false);
	void _init();
	void _ready();
};
