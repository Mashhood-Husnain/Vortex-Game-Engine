#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include "vortex_logs.hpp"
#include "vortex_global_vars.hpp"

class VortexKeyboard
{
    static GLFWwindow* s_window;
    static std::unordered_map<std::string, int> s_key_map;
    static std::unordered_map<std::string, bool> s_previous_keys;

public:
    static bool is_disabled;

    static void init(GLFWwindow* window);
    static void update();
    static bool get_key_down(const std::string& key, const float time_to_wait=0.0f, bool timer_active=false);
    static bool get_key(const std::string& key);
};
