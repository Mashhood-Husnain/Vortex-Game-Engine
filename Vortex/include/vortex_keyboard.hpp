#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include <iostream>
#include <GLFW/glfw3.h>

#include <imgui.h>

#include "vortex_logs.hpp"
#include "vortex_global_vars.hpp"

enum class Key
{
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    LEFT_CTRL, RIGHT_CTRL, LEFT_SHIFT, RIGHT_SHIFT, SPACE, ENTER, LEFT_ALT, RIGHT_ALT,
    TAB, ESCAPE, ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT, DELETE
};

struct KeyInfo
{
    int glfw_code;
    std::string name;
};

class VortexKeyboard
{
    static GLFWwindow *s_window;
    static std::unordered_map<Key, KeyInfo> s_key_map;
    static std::unordered_map<Key, bool> s_previous_keys;

    static bool is_modifier_key(const Key &key);
    static bool is_key_pressed(const Key &key);

public:
    static bool is_disabled;

    static void init(GLFWwindow *window);
    static void update();
    static bool get_key(const Key &key, const float time_to_wait=0.0f, bool timer_active=false);
    static std::string str_key(const Key &key);
};
