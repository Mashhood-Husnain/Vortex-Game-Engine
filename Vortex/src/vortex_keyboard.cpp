#include "vortex_keyboard.hpp"

GLFWwindow* VortexKeyboard::s_window = nullptr;
std::unordered_map<Key, KeyInfo> VortexKeyboard::s_key_map;
std::unordered_map<Key, bool> VortexKeyboard::s_previous_keys;

bool VortexKeyboard::is_disabled = false;


bool VortexKeyboard::is_modifier_key(const Key &key)
{
    return key == Key::LEFT_CTRL || key == Key::RIGHT_CTRL ||
           key == Key::LEFT_SHIFT || key == Key::RIGHT_SHIFT ||
           key == Key::LEFT_ALT;
}

bool VortexKeyboard::is_key_pressed(const Key &key)
{
    auto it = s_key_map.find(key);

    if (it == s_key_map.end())
        return false;

    return glfwGetKey(s_window, it->second.glfw_code) == GLFW_PRESS;
}

std::string VortexKeyboard::str_key(const Key &key)
{
    auto it = s_key_map.find(key);
    if (it == s_key_map.end())
    {
        VORTEX_WARN("[INPUT WARNING] Key not recognized!");
        return {};
    }

    return it->second.name;
}

void VortexKeyboard::init(GLFWwindow *window)
{
    s_window = window;

    if (s_key_map.empty())
    {
        s_key_map = {
            {Key::A, {GLFW_KEY_A, "A"}}, {Key::B, {GLFW_KEY_B, "B"}}, {Key::C, {GLFW_KEY_C, "C"}}, {Key::D, {GLFW_KEY_D, "D"}},
            {Key::E, {GLFW_KEY_E, "E"}}, {Key::F, {GLFW_KEY_F, "F"}}, {Key::G, {GLFW_KEY_G, "G"}}, {Key::H, {GLFW_KEY_H, "H"}},
            {Key::I, {GLFW_KEY_I, "I"}}, {Key::J, {GLFW_KEY_J, "J"}}, {Key::K, {GLFW_KEY_K, "K"}}, {Key::L, {GLFW_KEY_L, "L"}},
            {Key::M, {GLFW_KEY_M, "M"}}, {Key::N, {GLFW_KEY_N, "N"}}, {Key::O, {GLFW_KEY_O, "O"}}, {Key::P, {GLFW_KEY_P, "P"}},
            {Key::Q, {GLFW_KEY_Q, "Q"}}, {Key::R, {GLFW_KEY_R, "R"}}, {Key::S, {GLFW_KEY_S, "S"}}, {Key::T, {GLFW_KEY_T, "T"}},
            {Key::U, {GLFW_KEY_U, "U"}}, {Key::V, {GLFW_KEY_V, "V"}}, {Key::W, {GLFW_KEY_W, "W"}}, {Key::X, {GLFW_KEY_X, "X"}},
            {Key::Y, {GLFW_KEY_Y, "Y"}}, {Key::Z, {GLFW_KEY_Z, "Z"}},

            {Key::SPACE, {GLFW_KEY_SPACE, "Space"}},
            {Key::ENTER, {GLFW_KEY_ENTER, "Enter"}},
            {Key::LEFT_SHIFT, {GLFW_KEY_LEFT_SHIFT, "Left Shift"}},
            {Key::RIGHT_SHIFT,{GLFW_KEY_RIGHT_SHIFT, "Right Shift"}},
            {Key::LEFT_CTRL, {GLFW_KEY_LEFT_CONTROL, "Left Control"}},
            {Key::RIGHT_CTRL, {GLFW_KEY_RIGHT_CONTROL, "Right Control"}},
            {Key::LEFT_ALT, {GLFW_KEY_LEFT_ALT, "Left Alt"}},
            {Key::RIGHT_ALT,{GLFW_KEY_RIGHT_ALT, "Right Alt"}},
            {Key::TAB, {GLFW_KEY_TAB, "Tab"}},
            {Key::ESCAPE, {GLFW_KEY_ESCAPE, "Escape"}},
            {Key::ARROW_UP, {GLFW_KEY_UP, "Up Arrow"}},
            {Key::ARROW_DOWN, {GLFW_KEY_DOWN, "Down Arrow"}},
            {Key::ARROW_LEFT, {GLFW_KEY_LEFT, "Left Arrow"}},
            {Key::ARROW_RIGHT,{ GLFW_KEY_RIGHT, "Right Arrow"}},

            {Key::DELETE, {GLFW_KEY_DELETE, "Delete"}}
        };

        for (const auto& pair : s_key_map)
        {
            s_previous_keys[pair.first] = false;
        }
    }
}

void VortexKeyboard::update()
{
    if (!s_window) return;

    for (const auto& pair : s_key_map)
    {
        s_previous_keys[pair.first] = (glfwGetKey(s_window, pair.second.glfw_code) == GLFW_PRESS);
    }
}

bool VortexKeyboard::get_key(const Key &key, const float time_to_wait, bool timer_active)
{
    if (!s_window)
    {
        VORTEX_WARN("[INPUT WARNING] Keyboard not initialized!");
        return false;
    }

    if (ImGui::GetIO().WantTextInput)
    {
        is_disabled = true;
        return false;
    }

    is_disabled = false;

    auto it = s_key_map.find(key);

    if (it == s_key_map.end())
    {
        VORTEX_WARN("[INPUT WARNING] Key not recognized!");
        return false;
    }

    bool is_down_now = is_key_pressed(key);
    bool was_down_last_frame = s_previous_keys[key];

    if (is_modifier_key(key)) return is_down_now;

    static std::unordered_map<Key, float> key_timers;

    if (is_down_now)
    {
        if (!was_down_last_frame)
        {
            key_timers[key] = 0.0f;
            return true;
        }

        if (timer_active)
        {
            key_timers[key] += GLOBAL::deltaTime;

            if (key_timers[key] >= time_to_wait)
            {
                key_timers[key] = 0.0f;
                return true;
            }
        }
    }
    else
    {
        key_timers[key] = 0.0f;
    }

    return false;
}