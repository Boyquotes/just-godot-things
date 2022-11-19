#include "SpriteText.h"

/*******************************************************************************
SPRITE FONT CLASS ============================================================
Definition object instanced by Sprite Text objects.Setting character size and
character set will automatically cut the frames appropriately.
*******************************************************************************/
void SpriteFont::_register_methods()
{
	register_property("character_size", &SpriteFont::set_character_size, &SpriteFont::get_character_size, Vector2(8.0f, 8.0f));
	register_property("character_set", &SpriteFont::set_character_set, &SpriteFont::get_character_set, String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789., !? '\":()+-*/=@#$_"),
		GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_MULTILINE_TEXT);
}

void SpriteFont::set_character_size(Vector2 new_character_size)
{
	character_size = new_character_size;
	set_hframes(int(get_texture()->get_width() / character_size.x));
	set_vframes(int(get_texture()->get_height() / character_size.y));
}

Vector2 SpriteFont::get_character_size() { return character_size; }
void SpriteFont::set_character_set(String new_character_set) { character_set = new_character_set; }
String SpriteFont::get_character_set() { return character_set; }
void SpriteFont::_init() { set_character_size(character_size); }

/*******************************************************************************
SPRITE TEXT CLASS ============================================================
Object that writes and displays text.Used in conjunction with message windows,
menus, etc... Main information conveyance object.

Node Tree:
- ReferenceRect "text"
	- ReferenceRect "scroll"
*******************************************************************************/
void SpriteText::_register_methods()
{
	// Properties
	register_property("text_update", &SpriteText::set_text_update, &SpriteText::get_text_update, false);
	register_property("font", &SpriteText::set_font, &SpriteText::get_font, 0, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, 
		GODOT_PROPERTY_HINT_ENUM, "DEFAULT,CONSOLE");
	register_property("font_scale", &SpriteText::set_font_scale, &SpriteText::get_font_scale, Vector2::ONE);
	register_property("font_color", &SpriteText::set_font_color, &SpriteText::get_font_color, Color(1.0f,1.0f,1.0f,1.0f));
	register_property("font_shader", &SpriteText::set_font_shader, &SpriteText::get_font_shader, String(""));
	register_property("text", &SpriteText::set_text, &SpriteText::get_text, String(""), GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT, GODOT_PROPERTY_HINT_MULTILINE_TEXT);
	register_property("write_speed", &SpriteText::set_write_speed, &SpriteText::get_write_speed, -1.0f);
	register_property("alignment", &SpriteText::set_alignment, &SpriteText::get_alignment, 0, GODOT_METHOD_RPC_MODE_DISABLED, GODOT_PROPERTY_USAGE_DEFAULT,
		GODOT_PROPERTY_HINT_ENUM, "LEFT,CENTER,RIGHT");
	register_property("word_wrap", &SpriteText::set_word_wrap, &SpriteText::get_word_wrap, true);
	register_property("text_margin", &SpriteText::set_text_margin, &SpriteText::get_text_margin, Vector2(0.0f, 2.0f));
	register_property("auto_scroll", &SpriteText::set_auto_scroll, &SpriteText::get_auto_scroll, true);
	register_property("scroll_speed", &SpriteText::set_scroll_speed, &SpriteText::get_scroll_speed, 64.0f);
	
	// Methods
	register_method("get_font_res", &SpriteText::get_font_res);
	register_method("set_font_res", &SpriteText::set_font_res);
	register_method("set_font_shader_res", &SpriteText::set_font_shader_res);
	register_method("clear", &SpriteText::clear);
	register_method("write", &SpriteText::write);
	register_method("write_char", &SpriteText::write_char);

	// Base Processing
	register_method("_ready", &SpriteText::_ready);
	register_method("_process", &SpriteText::_process);

	// Signals
	register_signal<SpriteText>("write_complete");
}

// SETGETS -------------------------------------------------------------------------
void SpriteText::set_text_update(bool is_test)
{
	text_update = is_test;
	write(text);
	text_update = false;
}

bool SpriteText::get_text_update() { return text_update; }

// Font
void SpriteText::set_font(int new_font_index)
{
	font_index = new_font_index;
	set_font_res(font_sources[font_index]);
}

int SpriteText::get_font() { return font_index; }

void SpriteText::set_font_res(String new_font_res)
{
	font_res = ResourceLoader::get_singleton()->load(new_font_res);
	Sprite* c = cast_to<Sprite>(font_res->instance());
	font_size = c->get("character_size");
	c->free();
}

Ref<PackedScene> SpriteText::get_font_res() { return font_res; }
void SpriteText::set_font_scale(Vector2 s) { font_scale = s; }
Vector2 SpriteText::get_font_scale() { return font_scale; }
void SpriteText::set_font_color(Color new_font_color) { font_color = new_font_color; }
Color SpriteText::get_font_color() { return font_color; }
void SpriteText::set_font_shader(String new_font_shader) { font_shader = new_font_shader; }
String SpriteText::get_font_shader() { return font_shader; }
void SpriteText::set_font_shader_res(String new_shader_res) { font_shader_res = ResourceLoader::get_singleton()->load(new_shader_res); }

// Writing
void SpriteText::set_text(String new_text) { text = new_text; }
String SpriteText::get_text() { return text; }
void SpriteText::set_write_speed(float new_write_speed) { write_speed = new_write_speed; }
float SpriteText::get_write_speed() { return write_speed; }
void SpriteText::set_alignment(int new_alignment) { alignment = new_alignment; }
int SpriteText::get_alignment() { return alignment; }
void SpriteText::set_word_wrap(bool is_word_wrap) { word_wrap = is_word_wrap; }
bool SpriteText::get_word_wrap() { return word_wrap; }
void SpriteText::set_text_margin(Vector2 new_margin) { text_margin = new_margin; }
Vector2 SpriteText::get_text_margin() { return text_margin; }
void SpriteText::set_auto_scroll(bool is_auto_scroll) { auto_scroll = is_auto_scroll; }
bool SpriteText::get_auto_scroll() { return auto_scroll; }
void SpriteText::set_scroll_speed(float new_scroll_speed) { scroll_speed = new_scroll_speed; }
float SpriteText::get_scroll_speed() { return scroll_speed; }

// WRITE -------------------------------------------------------------------------------
void SpriteText::clear()
{
	if (is_queued_for_deletion())
		return;
	ReferenceRect* scr = cast_to<ReferenceRect>(get_node("scroll"));
	while (scr->get_child_count() > 0)
		scr->get_child(0)->free();
	text = "";
	write_pos = Vector2::ZERO;
	scr->set_size(Vector2(scr->get_size().x, font_size.y));
	scr->set_position(Vector2::ZERO);
}

void SpriteText::write(String new_text, bool clear_text)
{
	if (is_queued_for_deletion())
		return;
	if (clear_text)
		clear();
	set_font_res(font_sources[font_index]);
	text = new_text;
	write_progress = 0;
	write_ct = 0.0f;
	write_loop(get_process_delta_time(), cast_to<ReferenceRect>(get_node("scroll")));
}

void SpriteText::write_loop(float delta, ReferenceRect* scr)
{
	// Writing
	if (write_progress < text.length())
	{
		// Instant gratification
		if (write_speed < 0.0f)
		{
			for (int i = 0; i < text.length() - write_progress; i++)
				if (is_inside_tree())
					write_char(i + write_progress);
			write_progress = text.length();
		}
		// Typewriter effect
		else
		{
			if (write_ct <= 0)
			{
				write_ct = write_speed;
				write_char(write_progress);
				write_progress++;
			}
			else
				write_ct -= delta;
		};
	};

	// Finish writing
	if (write_progress == text.length())
	{
		write_progress++;
		emit_signal("write_complete");
	};
	if (scr->get_child_count() > MAX_CHARS)
		scr->get_child(0)->queue_free();
}

void SpriteText::write_char(int i)
{
	if (i >= text.length())
		return;
	float px = write_pos.x, py = write_pos.y;
	float fx = font_size.x * font_scale.x + text_margin.x, fy = font_size.y * font_scale.y + text_margin.y;
	ReferenceRect* scr = cast_to<ReferenceRect>(get_node("scroll"));
	// Create the character
	// Newline
	if (text[i] == L'\n')
	{
		py += 1;
		px = 0;
	}
	else if (text[i] == L'\\' && text[i + 1] == L'n')
	{
		py += 1;
		px = 0;
		write_progress++;
	}
	// Space
	else if (text[i] == L' ')
	{
		write_ct = 0.0f;
		px += 1;
		if (word_wrap)
		{
			if (text.find(" ", i + 1) == -1)
				next_word_length = text.length() - i - 1;
			else
				next_word_length = text.find(" ", i + 1) - i - 1;
			// We only wrap if we don't find a newline in the middle of our word
			if (text.find("\n", i + 1) == -1 || text.find("\n", i + 1) > i + next_word_length)
				if ((px + next_word_length) * fx >= get_size().x)
				{
					py += 1;
					px = 0;
				};
		};
		if (px > 0)
		{
			SpriteFont* c = cast_to<SpriteFont>(font_res->instance());
			c->hide();
			c->set_scale(font_scale);
			scr->add_child(c);
			c->set_owner(scr);
			c->set_position(Vector2(px * fx, py * fy));
		};
	}
	// Standard character
	else
	{
		SpriteFont* c = cast_to<SpriteFont>(font_res->instance());
		// Are we trying to parse a control hint?
		if (text[i] == L'$' && text[i + 1] == L'c')
		{
			c->set_frame(int(Math::clamp(int(text[i + 2]) * 10 + int(text[i + 3]), 0, 48)));
			write_progress += 3;
		}
		// Find the correct character in our text texture
		else
		{
			int frame = c->get_character_set().find(text[i]);
			if (frame >= 0)
				c->set_frame(Math::min(frame, int(c->get_hframes() * c->get_vframes())));
			else
				c->hide();
		};
		// Colorize the character
		c->set_modulate(font_color);
		// Obvious
		c->set_scale(font_scale);
		// We can apply a custom shader to the individual characters
		if (font_shader != "")
		{
			c->set_material(font_shader_res->duplicate());
			Ref<ShaderMaterial>((c)->get_material())->set_shader_param("char_index", i);
		};
		// Add the character to the tree
		scr->add_child(c);
		// Positioning is complicated, huh?
		if (float(px) * fx >= get_size().x)
		{
			py += 1;
			px = 0;
		};
		c->set_position(Vector2(px * fx, py * fy));
		if (alignment != ALIGN::LEFT)
		{
			int t_ct = int(scr->get_child_count()) - 1;
			for (int j = 0; j < px + 1; j++)
			{
				if (t_ct - j >= 0)
				{
					SpriteFont* align_c = cast_to<SpriteFont>(scr->get_child(t_ct - j));
					float x;
					if (alignment == ALIGN::CENTER)
					{
						x = scr->get_size().x * 0.5f + px * 0.5f * fx;
						x -= fx * j;
					}
					else if (alignment == ALIGN::RIGHT)
					{
						x = scr->get_size().x - fx + text_margin.x;
						x -= fx * j;
					}
					align_c->set_position(Vector2(x, py * fy));
				};
			};
		};
		px += 1;
	};
	scr->set_size(Vector2(scr->get_size().x, float(py + 1) * fy + 1.0f));
	write_pos = Vector2(px, py);
}

// BASE PROCESSING -----------------------------------------------------------------
void SpriteText::_init()
{
	font_sources.push_back("res://ui/spriteText/spriteFont.tscn");
	font_sources.push_back("res://ui/spriteText/utf_console.tscn");
}

void SpriteText::_ready()
{
	set_font_res(font_sources[font_index]);
	cast_to<ReferenceRect>(get_node("scroll"))->set_size(Vector2(get_size().x, get_size().y + 1.0f));
	call_deferred("write",text);
}

void SpriteText::_process(float delta)
{
	ReferenceRect* scr = cast_to<ReferenceRect>(get_node("scroll"));

	// Writing
	write_loop(delta, scr);

	// Scrolling
	scr->set_size(Vector2(get_size().x, scr->get_size().y));
	if (auto_scroll && scr->get_margin(3) > get_size().y)
	{
		if (scroll_speed > 0.0f)
			scr->set_position(Vector2(scr->get_position().x, fmaxf(scr->get_position().y - scroll_speed * delta, get_size().y - scr->get_size().y)));
		else
			scr->set_position(Vector2(scr->get_position().x, get_size().y - scr->get_size().y));
	};
}
