/***************************************************************************

CONTROLS MANAGER

Autoload singleton interface for user input. All input checks are requested
through this node.

***************************************************************************/
#include "ControlsMgr.h"
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>

using namespace godot;

void ControlsManager::_bind_methods()
{
    // PROPERTIES
    // mouse sensitivity
    ClassDB::bind_method(D_METHOD("set_mouse_sensitivity", "new_sensitivity"), &ControlsManager::set_mouse_sensitivity);
    ClassDB::bind_method(D_METHOD("get_mouse_sensitivity"), &ControlsManager::get_mouse_sensitivity);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::VECTOR2, "mouse_sensitivity"), "set_mouse_sensitivity", "get_mouse_sensitivity");
    // mouse motion
    ClassDB::bind_method(D_METHOD("set_mouse_motion", "new_mouse_motion"), &ControlsManager::set_mouse_motion);
    ClassDB::bind_method(D_METHOD("get_mouse_motion"), &ControlsManager::get_mouse_motion);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::VECTOR2, "mouse_motion"), "set_mouse_motion", "get_mouse_motion");
    // move motion
    ClassDB::bind_method(D_METHOD("set_move_motion", "new_move_motion"), &ControlsManager::set_move_motion);
    ClassDB::bind_method(D_METHOD("get_move_motion"), &ControlsManager::get_move_motion);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::VECTOR2, "move_motion"), "set_move_motion", "get_move_motion");
    // mouse y invert
    ClassDB::bind_method(D_METHOD("set_mouse_invert", "new_invert"), &ControlsManager::set_mouse_invert);
    ClassDB::bind_method(D_METHOD("get_mouse_invert"), &ControlsManager::get_mouse_invert);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::VECTOR2, "mouse_invert"), "set_mouse_invert", "get_mouse_invert");
    // gamepad y invert
    ClassDB::bind_method(D_METHOD("set_gamepad_invert", "new_invert"), &ControlsManager::set_gamepad_invert);
    ClassDB::bind_method(D_METHOD("get_gamepad_invert"), &ControlsManager::get_gamepad_invert);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::VECTOR2, "gamepad_invert"), "set_gamepad_invert", "get_gamepad_invert");
    // lockout
    ClassDB::bind_method(D_METHOD("set_lockout", "new_lockout"), &ControlsManager::set_lockout);
    ClassDB::bind_method(D_METHOD("get_lockout"), &ControlsManager::get_lockout);
    ClassDB::add_property("ControlsManager", PropertyInfo(Variant::FLOAT, "lockout"), "set_lockout", "get_lockout");
    ClassDB::bind_method(D_METHOD("locked"), &ControlsManager::locked);

    // INPUT MODE DETECTION
    ClassDB::bind_method(D_METHOD("_gamepad_connected"), &ControlsManager::_gamepad_connected);

    // INPUT STATES
    ClassDB::bind_method(D_METHOD("pressed", "action"), &ControlsManager::pressed);
    ClassDB::bind_method(D_METHOD("released", "action"), &ControlsManager::released);
    ClassDB::bind_method(D_METHOD("held", "action"), &ControlsManager::held);
    ClassDB::bind_method(D_METHOD("get_held_time", "action"), &ControlsManager::get_held_time);
    ClassDB::bind_method(D_METHOD("release_all"), &ControlsManager::release_all);

    // LOCKOUT
    ClassDB::bind_method(D_METHOD("mouse_lock", "locked"), &ControlsManager::mouse_lock);

    // SIGNALS
    ADD_SIGNAL(MethodInfo("console_input"));
    ADD_SIGNAL(MethodInfo("action_pressed", PropertyInfo(Variant::STRING, "action")));
    ADD_SIGNAL(MethodInfo("action_released", PropertyInfo(Variant::STRING, "action")));
    ADD_SIGNAL(MethodInfo("action_remapped"));

    // BASE PROCESSING
    ClassDB::bind_method(D_METHOD("input", "event"), &ControlsManager::input);
}

/*************************************************
PROPERTIES
*************************************************/
void ControlsManager::set_mouse_sensitivity(Vector2 new_sensitivity)
{
    mouse_sensitivity.x = Math::clamp(new_sensitivity.x, 0.01f, 1.0f);
    mouse_sensitivity.y = Math::clamp(new_sensitivity.y, 0.01f, 1.0f);
}
Vector2 ControlsManager::get_mouse_sensitivity() { return mouse_sensitivity; }

void ControlsManager::set_mouse_motion(Vector2 new_mouse_motion) { mouse_motion = new_mouse_motion; }
Vector2 ControlsManager::get_mouse_motion() { return mouse_motion; }

void ControlsManager::set_move_motion(Vector2 new_move_motion) { move_motion = new_move_motion; }
Vector2 ControlsManager::get_move_motion() { return move_motion; }

void ControlsManager::set_mouse_invert(Vector2 new_invert)
{
    if (new_invert.length() != 0)
        mouse_invert = new_invert.sign();
}
Vector2 ControlsManager::get_mouse_invert() { return mouse_invert; }

void ControlsManager::set_gamepad_invert(Vector2 new_invert)
{
    if (new_invert.length() != 0)
        gamepad_invert = new_invert.sign();
}
Vector2 ControlsManager::get_gamepad_invert() { return gamepad_invert; }

void ControlsManager::set_lockout(float new_lockout) { lockout = new_lockout; }
float ControlsManager::get_lockout() { return lockout; }
bool ControlsManager::locked()
{
    if (lockout > 0.0f)
        return true;
    return false;
}

/*************************************************
INPUT MODE DETECTION
*************************************************/
// Gamepad initialize
void ControlsManager::_gamepad_connected(int device_id, bool is_connected)
{
    if (is_connected)
    {
        if (INPUT->is_joy_known(device_id))
        {
            // Determine what kind of controller it is
            if (input_mode != InputMode::Keyboard)
            {
                String joy_name = INPUT->get_joy_name(device_id);
                if (joy_name.find("SNES") > -1)
                    input_mode = InputMode::SNES;
                else if (joy_name.find("PS4") > -1)
                    input_mode = InputMode::PSX;
                else
                    input_mode = InputMode::Xbox;
                //set_control_map();
            };
        };
    };
}

// Detect current input method; used for UI display mostly
void ControlsManager::input_mode_swap(InputEvent* event)
{
    if (input_mode == InputMode::Keyboard)
    {
        bool pad = false;
        if (event->get_class() == "InputEventJoypadButton")
            pad = true;
        // If we don't check the deadzone, it will always be putting out the JoypadMotion event
        if (event->get_class() == "InputEventJoypadMotion" && abs(float(event->call("get_axis_value"))) > DEADZONE)
            pad = true;
        if (pad)
        {
            String joy_name = INPUT->get_joy_name(0);
            if (joy_name.find("SNES") > -1)
                input_mode = InputMode::SNES;
            else if (joy_name.find("PS4") > -1)
                input_mode = InputMode::PSX;
            else
                input_mode = InputMode::Xbox;
            //set_control_map();
        };
    }
    else if (event->get_class() == "InputEventKey" || event->get_class() == "InputEventMouseButton")
        input_mode = InputMode::Keyboard;
}

/*************************************************
MAPPING
*************************************************/
void ControlsManager::set_control_map()
{
    InputMap* IM = InputMap::get_singleton();
    String control;
    int map_input[2]; // input
    //int last_input = gamepad_control_map[control][1];

    for (int i = 0; i < ACTIONS.size(); i++)
    {
        control = ACTIONS[i];
        IM->action_erase_events(control);

        for (int j = 0; j < 2; j++)
        {
            if (j == InputMode::Keyboard)
            {
                map_input[0] = keyboard_control_map[control.utf8().get_data()][0]; // input event type
                map_input[1] = keyboard_control_map[control.utf8().get_data()][1]; // input index
                switch (map_input[0])
                {
                case InputType::KEY:
                {
                    Ref<InputEventKey> event;
                    event.instantiate();
                    event->set_keycode(Key(map_input[1]));
                    IM->action_add_event(control, event);
                    break;
                }
                case InputType::MOUSEBUTTON:
                {
                    Ref<InputEventMouseButton> event;
                    event.instantiate();
                    event->set_button_index(MouseButton(map_input[1]));
                    /*
                    while (mouse_actions.size() < map_input[1])
                        mouse_actions.push_back("");
                    mouse_actions[map_input[1]] = control;
                    */
                    IM->action_add_event(control, event);
                    break;
                }
                }
            }
            else
            {
                map_input[0] = gamepad_control_map[control.utf8().get_data()][0]; // input event type
                map_input[1] = gamepad_control_map[control.utf8().get_data()][1]; // input index
                //if (i > 0)
                //    last_input = gamepad_control_map[ACTIONS[i - 1]][1];
                switch (map_input[0])
                {
                case InputType::JOYBUTTON:
                {
                    Ref<InputEventJoypadButton> event;
                    event.instantiate();
                    event->set_button_index(JoyButton(map_input[1]));
                    IM->action_add_event(control, event);
                    break;
                }
                case InputType::JOYAXIS:
                {
                    //if (control == "move_down" || control == "move_backward")
                    //    map_input[1] = last_input;
                    Ref<InputEventJoypadMotion> event;
                    event.instantiate();
                    event->set_axis(JoyAxis(map_input[1]));
                    IM->action_add_event(control, event);
                    IM->action_set_deadzone(control, DEADZONE);
                }
                }
            };
        };
    };
}

void ControlsManager::reset_to_defaults()
{
    keyboard_control_map = DEFAULT_KEYBOARD_MAP;
    gamepad_control_map = DEFAULT_GAMEPAD_MAP;
    mouse_sensitivity = Vector2(0.5f, 0.5f);
    mouse_invert = Vector2(1.0f, 1.0f);
    gamepad_invert = Vector2(1.0f, 1.0f);
    set_control_map();
}

void ControlsManager::set_remap_mode(String new_remap_action, bool new_remap_mode)
{
    remap_action = new_remap_action;
    remap_mode = new_remap_mode;
}

bool ControlsManager::remap(InputEvent* event, String action)
{
    String _class = event->get_class();

    // We don't accept mouse motion because it's more work than I feel like putting in right now, maybe later
    if (_class == "InputEventMouseMotion")
        return false;

    int new_input[2] = { -1, -1 }, input_check[2] = { -1, -1 };

    if (_class == "InputEventKey")
    {
        input_check[1] = event->call("get_keycode");
        if (input_check[1] != KEY_QUOTELEFT)
            if (input_check[1] < KEY_F1 || input_check[1] > KEY_F12)
            {
                new_input[0] = InputType::KEY;
                new_input[1] = input_check[1];
            }
    }
    else if (_class == "InputEventMouseButton")
    {
        new_input[0] = InputType::MOUSEBUTTON;
        new_input[1] = event->call("get_button_index");
        //while (new_input[1] >= mouse_actions.size())
        //    mouse_actions.push_back("");
    }
    else if (_class == "InputEventJoypadMotion")
    {
        input_check[1] = event->call("get_axis");
        // Joysticks are reserved for movement and aim, but the triggers are fair game
        if (input_check[1] > JOY_AXIS_RIGHT_Y)
        {
            new_input[0] = InputType::JOYAXIS;
            new_input[1] = input_check[1];
        }
    }
    else if (_class == "InputEventJoypadButton")
    {
        input_check[1] = event->call("get_button_index");
        if (input_check[1] != JOY_BUTTON_GUIDE && input_check[1] != JOY_BUTTON_MISC1)
        {
            new_input[0] = InputType::JOYBUTTON;
            new_input[1] = input_check[1];
        }
    }

    if (new_input[1] >= 0)
    {
        // Grab the right map
        std::unordered_map<std::string, std::vector<int>> current_control_map;
        if (input_mode == InputMode::Keyboard)
            current_control_map = keyboard_control_map;
        else
            current_control_map = gamepad_control_map;

        std::string _action = action.utf8().get_data();

        // We need to make sure we don't have 2 actions assigned to the same input
        input_check[0] = current_control_map[_action][0];
        input_check[1] = current_control_map[_action][1];
        for (std::unordered_map<std::string, std::vector<int>>::iterator itr = current_control_map.begin(); itr != current_control_map.end(); itr++)
        {
            if (itr->second[1] == new_input[1] && itr->second[0] == new_input[0])
            {
                current_control_map[itr->first][0] = input_check[0];
                current_control_map[itr->first][1] = input_check[1];
                break;
            }
        }
        /*
        if (new_input[0] == MOUSEBUTTON)
        {
            // Make sure we don't have both mouse buttons assigned to the same input
            for (int i = 0; i < mouse_actions.size(); i++)
                if (mouse_actions[i] == action)
                    mouse_actions[i] = mouse_actions[new_b];
            // Setting up the new mouse input
            mouse_actions[new_b] = action;
        }
        */
        
        // Setting up the new control
        current_control_map[_action][0] = new_input[0];
        current_control_map[_action][1] = new_input[1];
        // Return the modified map to the appropriate map
        if (input_mode == InputMode::Keyboard)
            keyboard_control_map = current_control_map;
        else
            gamepad_control_map = current_control_map;
        
        // Remap Controls
        set_control_map();
        //get_node("/root/SoundManager")->call("menu_confirm");
        release_all();
        return true;
    }
    // We couldn't remap it?
    //get_node("/root/SoundManager")->call("menu_error");
    release_all();
    return false;
}

void ControlsManager::dict_to_map(int mode, Dictionary new_map)
{
    std::unordered_map<std::string, std::vector<int>> control_map;
    Array keys = new_map.keys(), v;
    for (int i = 0; i < keys.size(); i++)
    {
        v = new_map[keys[i]];
        control_map[String(keys[i]).utf8().get_data()] = {v[0], v[1]};
    };
    if (input_mode == InputMode::Keyboard)
        keyboard_control_map = control_map;
    else
        gamepad_control_map = control_map;
}

Dictionary ControlsManager::map_to_dict(int mode)
{
    std::unordered_map<std::string, std::vector<int>> control_map;
    if (input_mode == InputMode::Keyboard)
        control_map = keyboard_control_map;
    else
        control_map = gamepad_control_map;
    Dictionary dict_map = {};
    for (std::unordered_map<std::string, std::vector<int>>::iterator itr = control_map.begin(); itr != control_map.end(); itr++)
        dict_map[String(itr->first.c_str())] = Array::make(itr->second[0], itr->second[1]);
    return dict_map;
}

String ControlsManager::action_to_ui(String action, int mode)
{
    std::string _action = action.utf8().get_data();

    int ui_input[2] = { -1 };
    if (mode < 0)
        mode = input_mode;
    if (mode == InputMode::Keyboard)
    {
        ui_input[0] = keyboard_control_map[_action][0];
        ui_input[1] = keyboard_control_map[_action][1];
    }
    else
    {
        ui_input[0] = gamepad_control_map[_action][0];
        ui_input[1] = gamepad_control_map[_action][1];
    }

    if (ui_input[0] == InputType::KEY)
        return OS::get_singleton()->get_keycode_string(Key(ui_input[1]));

    if (ui_input[0] == InputType::MOUSEBUTTON)
    {
        switch (ui_input[1])
        {
        case MOUSE_BUTTON_LEFT:
            return "Mouse Left";
        case MOUSE_BUTTON_RIGHT:
            return "Mouse Right";
        case MOUSE_BUTTON_MIDDLE:
            return "Mouse Middle";
        case MOUSE_BUTTON_WHEEL_UP:
            return "Mouse Wheel Up";
        case MOUSE_BUTTON_WHEEL_DOWN:
            return "Mouse Wheel Down";
        case MOUSE_BUTTON_WHEEL_RIGHT:
            return "Mouse Wheel Right";
        case MOUSE_BUTTON_WHEEL_LEFT:
            return "Mouse Wheel Left";
        case MOUSE_BUTTON_XBUTTON1:
            return "Mouse X Button 1";
        case MOUSE_BUTTON_XBUTTON2:
            return "Mouse X Button 2";
        }
    }

    if (ui_input[0] == InputType::JOYBUTTON)
    {
        return "";
    }

    if (ui_input[0] == InputType::JOYAXIS)
    {
        return "";
    }
}

/*************************************************
INPUT STATES
*************************************************/
bool ControlsManager::pressed(String action) { return (lockout <= 0.0f && INPUT->is_action_just_pressed(action)); }

bool ControlsManager::released(String action) { return (lockout <= 0.0f && INPUT->is_action_just_released(action)); }

bool ControlsManager::held(String action) { return (lockout <= 0.0f && INPUT->is_action_pressed(action)); }

void ControlsManager::update_held_time(float delta)
{
    std::string c;
    for (int i = 0; i < ACTIONS.size(); i++)
    {
        c = ACTIONS[i].utf8().get_data();
        if (held(ACTIONS[i]))
            held_time[c] = float(held_time[c]) + delta;
        else
            held_time[c] = 0.0f;
    };
}

float ControlsManager::get_held_time(String action)
{
    return held_time[action.utf8().get_data()];
}

void ControlsManager::release_all()
{
    for (int i = 0; i < ACTIONS.size(); i++)
        INPUT->action_release(ACTIONS[i]);
}

void ControlsManager::mouse_lock(bool locked)
{
    if (locked)
        INPUT->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
    else
        INPUT->set_mouse_mode(Input::MOUSE_MODE_HIDDEN);
}

/*************************************************
BASE PROCESSING
*************************************************/
ControlsManager::ControlsManager()
{
	set_process_mode(Node::PROCESS_MODE_ALWAYS);
    keyboard_control_map = DEFAULT_KEYBOARD_MAP;
    gamepad_control_map = DEFAULT_GAMEPAD_MAP;
}

ControlsManager::~ControlsManager() {}

void ControlsManager::ready()
{
    if (Engine::get_singleton()->is_editor_hint())
        return;

    INPUT = Input::get_singleton();
    INPUT->connect("joy_connection_changed", Callable(this, "_gamepad_connected"));
    InputMap* inmap = InputMap::get_singleton();
    for (int i = 0; i < ACTIONS.size(); i++)
        inmap->add_action(ACTIONS[i]);
    set_control_map();
    //mouse_lock(true);
}

void ControlsManager::process(float delta)
{
    if (Engine::get_singleton()->is_editor_hint())
        return;

    if (input_mode == InputMode::Keyboard)
        set_deferred("mouse_motion", Vector2());
    update_held_time(delta);
    if (lockout > 0)
    {
        release_all();
        lockout -= delta;
    };
}

void ControlsManager::input(InputEvent* event)
{
    if (Engine::get_singleton()->is_editor_hint())
        return;

    // Dev console eats inputs
    if (console_mode)
    {
        if (event->get_class() == "InputEventKey")
            emit_signal("console_input", event);
        get_viewport()->set_input_as_handled();
        return;
    };

    // Input remapping eats inputs
    if (remap_mode)
    {
        if (remap(event, remap_action))
        {
            remap_mode = false;
            emit_signal("action_remapped");
        }
        get_viewport()->set_input_as_handled();
        return;
    };

    input_mode_swap(event);

    // Mouselook
    if (event->get_class() == "InputEventMouseMotion" && input_mode == InputMode::Keyboard)
        mouse_motion = Vector2(event->get("relative")) * mouse_sensitivity;
    
    // Gamepad movement
    else if (event->get_class() == "InputEventJoypadMotion" && input_mode != InputMode::Keyboard)
    {
        int axis = event->get("axis");
        float av = event->get("axis_value");
        //av = (abs(av) - DEADZONE) / (1.0 - DEADZONE) * Math::sign(av);
        // Movement
        switch (axis)
        {
        case (JoyAxis::JOY_AXIS_LEFT_X):
        {
            if (abs(av) > 0.0f)
                move_motion.x = av;
            else
                move_motion.x = 0.0f;
            break;
        }
        case (JoyAxis::JOY_AXIS_LEFT_Y):
        {
            if (abs(av) > 0.0f)
                move_motion.y = av;
            else
                move_motion.y = 0.0f;
            break;
        }
        // Aiming
        case (JoyAxis::JOY_AXIS_RIGHT_X):
        {
            if (abs(av) > 0.0f)
                mouse_motion.x = av * 3.0f * mouse_sensitivity.x;
            else
                mouse_motion.x = 0.0f;
            break;
        }
        case (JoyAxis::JOY_AXIS_RIGHT_Y):
        {
            if (abs(av) > 0.0f)
                mouse_motion.y = av * 3.0f * mouse_sensitivity.y;
            else
                mouse_motion.y = 0.0f;
            break;
        };
        };
    };
    
    // Input states
    for (int i = 0; i < ACTIONS.size(); i++)
    {
        if (event->is_action_pressed(ACTIONS[i]))
        {
            emit_signal("action_pressed", ACTIONS[i]);
            break;
        }
        if (event->is_action_released(ACTIONS[i]))
        {
            emit_signal("action_released", ACTIONS[i]);
            break;
        };
    };
}

void ControlsManager::_notification(int _notif) {
    switch (_notif)
    {
    case NOTIFICATION_READY:
        ready();
        set_process(true);
        break;
    case NOTIFICATION_PROCESS:
        process(get_process_delta_time());
        break;
    }
}
