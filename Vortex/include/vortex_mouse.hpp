#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "vortex_logs.hpp"

class VortexMouse
{
    static GLFWwindow *s_window;
    static std::unordered_map<std::string, int> s_button_map;
    static std::unordered_map<std::string, bool> s_previous_buttons;

    static double s_mouse_x;
    static double s_mouse_y;

    static double s_previous_mouse_x;
    static double s_previous_mouse_y;

    static double s_scroll_accum_x;
    static double s_scroll_accum_y;
    static double s_scroll_x;
    static double s_scroll_y;
    static GLFWscrollfun s_previous_scroll_callback;

    static void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);
public:
    static void init(GLFWwindow *window);
    static void update();
    static bool get_button_down(const std::string &button);
    static bool get_button(const std::string &button);

    static double get_x();
    static double get_delta_x();

    static double get_y();
    static double get_delta_y();
    static glm::vec2 get_position();

    static double get_scroll();
    static double get_scroll_x();
};
