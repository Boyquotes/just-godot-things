#pragma once
#include "Godot.hpp"
#include <Math.hpp>
#include <ResourceLoader.hpp>
#include <PackedScene.hpp>
#include <string>
#include <vector>
#include <Texture.hpp>
#include "Sprite.hpp"
#include <ShaderMaterial.hpp>
#include "ReferenceRect.hpp"

/*******************************************************************************
SPRITE FONT CLASS ============================================================
Definition object instanced by Sprite Text objects.Setting character size and
character set will automatically cut the frames appropriately.
*******************************************************************************/
class SpriteFont : public Sprite
{
	GODOT_CLASS(SpriteFont, Sprite);
private:
	Vector2 character_size = Vector2(8.0f, 8.0f);
	String character_set = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789., !? '\":()+-*/=@#$_";
public:
	static void _register_methods();
	void set_character_size(Vector2 new_size);
	Vector2 get_character_size();
	void set_character_set(String new_character_set);
	String get_character_set();
	void _init();
};

/*******************************************************************************
SPRITE TEXT CLASS ============================================================
Object that writes and displays text.Used in conjunction with message windows,
menus, etc... Main information conveyance object.

Node Tree:
- ReferenceRect "text"
	- ReferenceRect "scroll"
*******************************************************************************/
class SpriteText : public ReferenceRect
{
private:
	GODOT_CLASS(SpriteText, ReferenceRect);
	const int MAX_CHARS = 1024;
public:
	// PROPERTIES
	bool text_update = false;
	// Font
	enum FONT_TYPE {CONSOLE,HUD_NO,POSTER,CREEP,GAMEPAD};
	int font_index = 0;
	std::vector<String> font_sources;
	Ref<PackedScene> font_res;
	Vector2 font_scale = Vector2::ONE, font_size = Vector2(8.0f, 8.0f);
	Color font_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	String font_shader = "";
	Ref<Material> font_shader_res;
	// Writing Setup
	String text = "";
	float write_speed = -1.0f, scroll_speed = 64.0f;
	enum ALIGN {LEFT, CENTER, RIGHT};
	int alignment = ALIGN::LEFT;
	bool word_wrap = true, auto_scroll = true;
	Vector2 text_margin = Vector2(0.0f, 2.0f);
	// Writing Handler
	int write_progress = 0, next_word_length = 0;
	float write_ct = 0.0f;
	Vector2 write_pos = Vector2::ZERO;
	// METHODS -----------------------------------------------------
	// Godot
	static void _register_methods();
	// SETGETS
	void set_text_update(bool is_test);
	bool get_text_update();
	// Font
	void set_font(int new_font_index);
	int get_font();
	void set_font_res(String new_font_res);
	Ref<PackedScene> get_font_res();
	void set_font_scale(Vector2 s);
	Vector2 get_font_scale();
	void set_font_color(Color new_font_color);
	Color get_font_color();
	void set_font_shader(String new_font_shader);
	String get_font_shader();
	void set_font_shader_res(String new_shader_res);
	// Writing
	void set_text(String new_text);
	String get_text();
	void set_write_speed(float new_write_speed);
	float get_write_speed();
	void set_alignment(int new_alignment);
	int get_alignment();
	void set_word_wrap(bool is_word_wrap);
	bool get_word_wrap();
	void set_text_margin(Vector2 new_margin);
	Vector2 get_text_margin();
	void set_auto_scroll(bool is_auto_scroll);
	bool get_auto_scroll();
	void set_scroll_speed(float new_scroll_speed);
	float get_scroll_speed();
	// Write
	void clear();
	void write(String new_text, bool clear_text = true);
	void write_loop(float delta, ReferenceRect* scr);
	void write_char(int i);
	// Base Processing
	void _init();
	void _ready();
	void _process(float delta);
};
