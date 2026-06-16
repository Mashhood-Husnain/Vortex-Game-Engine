#include "vortex_keyboard.hpp"

GLFWwindow* VortexKeyboard::s_window = nullptr;
std::unordered_map<std::string, int> VortexKeyboard::s_key_map;
std::unordered_map<std::string, bool> VortexKeyboard::s_previous_keys;

bool VortexKeyboard::is_disabled = false;

void VortexKeyboard::init(GLFWwindow* window)
{
    s_window = window;

    if (s_key_map.empty())
    {
        s_key_map = {
            {"A", GLFW_KEY_A}, {"B", GLFW_KEY_B}, {"C", GLFW_KEY_C}, {"D", GLFW_KEY_D},
            {"E", GLFW_KEY_E}, {"F", GLFW_KEY_F}, {"G", GLFW_KEY_G}, {"H", GLFW_KEY_H},
            {"I", GLFW_KEY_I}, {"J", GLFW_KEY_J}, {"K", GLFW_KEY_K}, {"L", GLFW_KEY_L},
            {"M", GLFW_KEY_M}, {"N", GLFW_KEY_N}, {"O", GLFW_KEY_O}, {"P", GLFW_KEY_P},
            {"Q", GLFW_KEY_Q}, {"R", GLFW_KEY_R}, {"S", GLFW_KEY_S}, {"T", GLFW_KEY_T},
            {"U", GLFW_KEY_U}, {"V", GLFW_KEY_V}, {"W", GLFW_KEY_W}, {"X", GLFW_KEY_X},
            {"Y", GLFW_KEY_Y}, {"Z", GLFW_KEY_Z},

            {"SPACE", GLFW_KEY_SPACE},
            {"ENTER", GLFW_KEY_ENTER},
            {"LEFTSHIFT", GLFW_KEY_LEFT_SHIFT},
            {"RIGHTSHIFT", GLFW_KEY_RIGHT_SHIFT},
            {"LEFTCONTROL", GLFW_KEY_LEFT_CONTROL},
            {"RIGHTCONTROL", GLFW_KEY_RIGHT_CONTROL},
            {"LEFTALT", GLFW_KEY_LEFT_ALT},
            {"RIGHTALT", GLFW_KEY_RIGHT_ALT},
            {"TAB", GLFW_KEY_TAB},
            {"ESCAPE", GLFW_KEY_ESCAPE},
            {"UP", GLFW_KEY_UP},
            {"DOWN", GLFW_KEY_DOWN},
            {"LEFT", GLFW_KEY_LEFT},
            {"RIGHT", GLFW_KEY_RIGHT},

            {"DELETE", GLFW_KEY_DELETE}
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

bool VortexKeyboard::get_key_down(const std::string& key, const float time_to_wait, bool timer_active)
{
    if (!s_window)
    {
        VORTEX_WARN("[INPUT WARNING] Keyboard not initialized! Call VortexKeyboard::init(window) first.");
        return false;
    }

    if (ImGui::GetIO().WantTextInput)
    {
        is_disabled = true;
        return false;
    }
    else
    {
        is_disabled = false;
    }

    auto it = s_key_map.find(key);
    if (it == s_key_map.end())
    {
        VORTEX_WARN("[INPUT WARNING] Key '", key, "' is not recognized!");
        return false;
    }

    bool is_down_now = (glfwGetKey(s_window, it->second) == GLFW_PRESS);
    bool was_down_last_frame = s_previous_keys[key];

    static std::unordered_map<std::string, float> key_timers;

    if (is_down_now)
    {
        if (!was_down_last_frame)
        {
            key_timers[key] = 0.0f;
            return true;
        }
        else if (timer_active)
        {
            key_timers[key] += GLOBAL::deltaTime;

            if (key_timers[key] >= time_to_wait)
            {
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

bool VortexKeyboard::get_key(const std::string& key)
{
    if (!s_window)
    {
        VORTEX_WARN("[INPUT WARNING] Keyboard not initialized! Call VortexKeyboard::init(window) first.");
        return false;
    }

    if (ImGui::GetIO().WantTextInput)
    {
        is_disabled = true;
        return false;
    }
    else
    {
        is_disabled = false;
    }

    auto it = s_key_map.find(key);

    if (it != s_key_map.end())
    {
        return (glfwGetKey(s_window, it->second) == GLFW_PRESS);
    }

    VORTEX_WARN("[INPUT WARNING] Key '", key, "' is not recognized!");

    return false;
}
