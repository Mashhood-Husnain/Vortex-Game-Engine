#include "vortex_keyboard.hpp"

GLFWwindow* VortexKeyboard::s_window = nullptr;
std::unordered_map<Key, int> VortexKeyboard::s_key_map;
std::unordered_map<Key, bool> VortexKeyboard::s_previous_keys;

bool VortexKeyboard::is_disabled = false;


bool VortexKeyboard::is_modifier_key(const Key& key)
{
    return key == Key::LEFT_CTRL || key == Key::RIGHT_CTRL ||
           key == Key::LEFT_SHIFT || key == Key::RIGHT_SHIFT ||
           key == Key::LEFT_ALT;
}

bool VortexKeyboard::is_key_pressed(const Key& key)
{
    auto it = s_key_map.find(key);

    if (it == s_key_map.end())
        return false;

    return glfwGetKey(s_window, it->second) == GLFW_PRESS;
}

void VortexKeyboard::init(GLFWwindow* window)
{
    s_window = window;

    if (s_key_map.empty())
    {
        s_key_map = {
            {Key::A, GLFW_KEY_A}, {Key::B, GLFW_KEY_B}, {Key::C, GLFW_KEY_C}, {Key::D, GLFW_KEY_D},
            {Key::E, GLFW_KEY_E}, {Key::F, GLFW_KEY_F}, {Key::G, GLFW_KEY_G}, {Key::H, GLFW_KEY_H},
            {Key::I, GLFW_KEY_I}, {Key::J, GLFW_KEY_J}, {Key::K, GLFW_KEY_K}, {Key::L, GLFW_KEY_L},
            {Key::M, GLFW_KEY_M}, {Key::N, GLFW_KEY_N}, {Key::O, GLFW_KEY_O}, {Key::P, GLFW_KEY_P},
            {Key::Q, GLFW_KEY_Q}, {Key::R, GLFW_KEY_R}, {Key::S, GLFW_KEY_S}, {Key::T, GLFW_KEY_T},
            {Key::U, GLFW_KEY_U}, {Key::V, GLFW_KEY_V}, {Key::W, GLFW_KEY_W}, {Key::X, GLFW_KEY_X},
            {Key::Y, GLFW_KEY_Y}, {Key::Z, GLFW_KEY_Z},

            {Key::SPACE, GLFW_KEY_SPACE},
            {Key::ENTER, GLFW_KEY_ENTER},
            {Key::LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT},
            {Key::RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT},
            {Key::LEFT_CTRL, GLFW_KEY_LEFT_CONTROL},
            {Key::RIGHT_CTRL, GLFW_KEY_RIGHT_CONTROL},
            {Key::LEFT_ALT, GLFW_KEY_LEFT_ALT},
            {Key::RIGHT_ALT, GLFW_KEY_RIGHT_ALT},
            {Key::TAB, GLFW_KEY_TAB},
            {Key::ESCAPE, GLFW_KEY_ESCAPE},
            {Key::ARROW_UP, GLFW_KEY_UP},
            {Key::ARROW_DOWN, GLFW_KEY_DOWN},
            {Key::ARROW_LEFT, GLFW_KEY_LEFT},
            {Key::ARROW_RIGHT, GLFW_KEY_RIGHT},

            {Key::DELETE, GLFW_KEY_DELETE}
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
        s_previous_keys[pair.first] = (glfwGetKey(s_window, pair.second) == GLFW_PRESS);
    }
}

bool VortexKeyboard::get_key(const Key& key, const float time_to_wait, bool timer_active)
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