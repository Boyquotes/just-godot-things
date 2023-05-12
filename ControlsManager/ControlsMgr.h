#ifndef BKG_CONTROLSMANAGER_H
#define BKG_CONTROLSMANAGER_H

/********************************************************************************
CONTROLS MANAGER
********************************************************************************/
#include <string>
#include <unordered_map>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

#define DEADZONE 0.333f

namespace godot
{
    class ControlsManager : public Node
    {
        GDCLASS(ControlsManager, Node)
    private:
        static void _bind_methods();
    public:
        enum InputMode { Keyboard, Xbox, PSX, SNES };

        Input* INPUT = nullptr;

        const std::vector<String> ACTIONS =
        {
            "up",
            "down",
            "right",
            "left",
            "accept",
            "cancel",
            "menu",
            "jump",
            "switch",
            "tool",
            "console"
        };

        enum InputType { KEY, MOUSEBUTTON, MOUSEAXIS, JOYBUTTON, JOYAXIS };

        const std::unordered_map<std::string, std::vector<int>> DEFAULT_KEYBOARD_MAP =
        {
            {"up", { KEY, KEY_W }},
            {"down", { KEY, KEY_S }},
            {"right", { KEY, KEY_D }},
            {"left", { KEY, KEY_A }},
            {"accept", { KEY, KEY_E }},
            {"cancel", { KEY, KEY_SHIFT }},
            {"jump", { KEY, KEY_SPACE }},
            {"switch", { KEY, KEY_R }},
            {"tool", { KEY, KEY_F }},
            {"menu", { KEY, KEY_Q }},
            {"console", { KEY, KEY_QUOTELEFT }}
        };

        const std::unordered_map<std::string, std::vector<int>> DEFAULT_GAMEPAD_MAP =
        {
            {"up", { JOYAXIS, JOY_AXIS_LEFT_Y }},
            {"down", { JOYAXIS, JOY_AXIS_LEFT_Y }},
            {"right", { JOYAXIS, JOY_AXIS_LEFT_X }},
            {"left", { JOYAXIS, JOY_AXIS_LEFT_X }},
            {"accept", { JOYBUTTON, JOY_BUTTON_B }},
            {"cancel", { JOYBUTTON, JOY_BUTTON_A }},
            {"jump", { JOYBUTTON, JOY_BUTTON_Y }},
            {"switch", { JOYAXIS, JOY_AXIS_TRIGGER_LEFT }},
            {"tool", { JOYBUTTON, JOY_BUTTON_LEFT_STICK }},
            {"menu", { JOYBUTTON, JOY_BUTTON_BACK }},
            {"console", { KEY, KEY_QUOTELEFT }}
        };

        std::unordered_map<std::string, std::vector<int>> keyboard_control_map = {}, gamepad_control_map = {};
        //std::vector<String> mouse_actions = { "", "attack", "alt_attack", "", "", "", "", "", "" };

        Dictionary map = {};
        int8_t input_mode = 0;
        Vector2 mouse_invert = Vector2(1.0f, 1.0f);
        Vector2 gamepad_invert = Vector2(1.0f, 1.0f);
        float lockout = 0.0f;
        bool remap_mode = false;
        bool console_mode = false;
        String remap_action;
        Vector2 mouse_motion = Vector2();
        Vector2 mouse_sensitivity = Vector2(0.5f, 0.5f);
        Vector2 move_motion = Vector2();
        std::unordered_map<std::string, float> held_time =
        {
            {"up", 0.0f },
            {"down", 0.0f },
            {"right", 0.0f },
            {"left", 0.0f },
            {"accept", 0.0f },
            {"cancel", 0.0f },
            {"jump", 0.0f },
            {"switch", 0.0f },
            {"tool", 0.0f },
            {"menu", 0.0f },
            {"console", 0.0f }
        };

        ControlsManager();
        ~ControlsManager();
        // PROPERTIES
        void set_mouse_sensitivity(Vector2 new_sensitivity = Vector2(0.5f, 0.5f)); Vector2 get_mouse_sensitivity();
        void set_mouse_motion(Vector2 new_mouse_motion); Vector2 get_mouse_motion();
        void set_move_motion(Vector2 new_move_motion); Vector2 get_move_motion();
        void set_mouse_invert(Vector2 new_invert = Vector2(1.0f, 1.0f)); Vector2 get_mouse_invert();
        void set_gamepad_invert(Vector2 new_invert = Vector2(1.0f, 1.0f)); Vector2 get_gamepad_invert();
        void set_lockout(float new_lockout); float get_lockout(); bool locked();

        // INPUT MODE DETECTION
        void _gamepad_connected(int device_id, bool is_connected);
        void input_mode_swap(InputEvent* event);

        // MAPPING
        void set_control_map();
        void reset_to_defaults();
        void set_remap_mode(String new_remap_action, bool new_remap_mode = true);
        bool remap(InputEvent* event, String action);
        void dict_to_map(int mode, Dictionary new_map);
        Dictionary map_to_dict(int mode);
        String action_to_ui(String action, int mode = -1);

        // INPUT STATES
        bool pressed(String action);
        bool released(String action);
        bool held(String action);
        void update_held_time(float delta);
        float get_held_time(String action);

        // LOCKOUT
        void release_all();
        void mouse_lock(bool locked);

        // BASE PROCESSING
        void ready();
        void process(float delta);
        void input(InputEvent* event);
        void _notification(int _notif);
    };
};

#endif // !BKG_CONTROLSMANAGER_H
