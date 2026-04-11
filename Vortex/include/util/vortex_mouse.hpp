#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>

class VortexMouse
{
    inline static GLFWwindow *s_window = nullptr;
    inline static std::unordered_map<std::string, int> s_button_map;
    inline static std::unordered_map<std::string, bool> s_previous_buttons;

    inline static double s_mouse_x = 0.0;
    inline static double s_mouse_y = 0.0;
public:
    static void init(GLFWwindow *window)
    {
        s_window = window;

        if (s_button_map.empty())
        {
            s_button_map = {
                {"LEFT", GLFW_MOUSE_BUTTON_LEFT},
                {"RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
                {"MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE}
            };

            for (const auto &pair : s_button_map)
            {
                s_previous_buttons[pair.first] = false;
            }
        }
    }

    static void update()
    {
        if (!s_window) return;

        for (const auto &pair : s_button_map)
        {
            s_previous_buttons[pair.first] = (glfwGetMouseButton(s_window, pair.second) == GLFW_PRESS);
        }

        glfwGetCursorPos(s_window, &s_mouse_x, &s_mouse_y);
    }

    static bool get_button_down(const std::string &button)
    {
        if (!s_window) return false;

        auto it = s_button_map.find(button);
        if (it != s_button_map.end())
        {
            bool is_down_now = (glfwGetMouseButton(s_window, it->second) == GLFW_PRESS);
            bool was_down_last_frame = s_previous_buttons[button];

            return is_down_now && !was_down_last_frame;
        }

        std::cerr << "[INPUT WARNING] Mouse Button '" << button << "' is not recognized!" << std::endl;
        return false;
    }

    static bool get_button(std::string &button)
    {
        if (!s_window) return false;

        auto it = s_button_map.find(button);
        if (it != s_button_map.end())
        {
            return (glfwGetMouseButton(s_window, it->second));
        }

        return false;
    }

    static double get_x() {return s_mouse_x;}
    static double get_y() {return s_mouse_y;}
};
