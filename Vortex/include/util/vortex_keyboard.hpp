#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>

class VortexKeyboard
{
private:
    inline static GLFWwindow* s_window = nullptr;
    inline static std::unordered_map<std::string, int> s_key_map;
    inline static std::unordered_map<std::string, bool> s_previous_keys;

public:
    static void init(GLFWwindow* window)
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
                {"ESCAPE", GLFW_KEY_ESCAPE},
                {"UP", GLFW_KEY_UP},
                {"DOWN", GLFW_KEY_DOWN},
                {"LEFT", GLFW_KEY_LEFT},
                {"RIGHT", GLFW_KEY_RIGHT}
            };

            for (const auto& pair : s_key_map) 
            {
                s_previous_keys[pair.first] = false;
            }
        }
    }

    static void update()
    {
        if (!s_window) return;

        for (const auto& pair : s_key_map) 
        {
            s_previous_keys[pair.first] = (glfwGetKey(s_window, pair.second) == GLFW_PRESS);
        }
    }

    static bool get_key_down(const std::string& key)
    {
        if (!s_window) return false;

        auto it = s_key_map.find(key);
        if (it != s_key_map.end())
        {
            bool is_down_now = (glfwGetKey(s_window, it->second) == GLFW_PRESS);
            bool was_down_last_frame = s_previous_keys[key];
            
            // It's a valid press ONLY if it wasn't already held down last frame!
            return is_down_now && !was_down_last_frame;
        }

        std::cerr << "[INPUT WARNING] Key '" << key << "' is not recognized!" << std::endl;
        return false;
    }

    static bool get_key(const std::string& key)
    {
        if (!s_window) 
        {
            std::cerr << "[INPUT ERROR] Keyboard not initialized! Call VortexKeyboard::init(window) first." << std::endl;
            return false;
        }

        auto it = s_key_map.find(key);
        
        if (it != s_key_map.end())
        {
            return (glfwGetKey(s_window, it->second) == GLFW_PRESS);
        }

        std::cerr << "[INPUT WARNING] Key '" << key << "' is not recognized!" << std::endl;
        return false;
    }
};
