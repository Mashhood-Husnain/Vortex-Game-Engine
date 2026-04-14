#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>

class VortexMouse
{
    static GLFWwindow *s_window;
    static std::unordered_map<std::string, int> s_button_map;
    static std::unordered_map<std::string, bool> s_previous_buttons;

    static double s_mouse_x;
    static double s_mouse_y;
public:
    static void init(GLFWwindow *window);
    static void update();
    static bool get_button_down(const std::string &button);
    static bool get_button(const std::string &button);

    static double get_x();
    static double get_y();
};
