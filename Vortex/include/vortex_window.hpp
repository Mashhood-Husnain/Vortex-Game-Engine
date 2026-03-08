#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <functional>

#include "vortex_camera.hpp"
#include "vortex_shaders.hpp"

#include "util/vortex_global_vars.hpp"

class VortexWindow
{
    GLFWwindow *window = nullptr;
    int default_window_width = 800;
    int default_window_height = 600;
    std::string window_name;
    bool is_fullscreen = false;
    int stored_window_x_pos;
    int stored_window_y_pos;
    int stored_window_width;
    int stored_window_height;
    VortexCamera *camera = nullptr;
    float mouse_last_x = default_window_width / 2;
    float mouse_last_y = default_window_height / 2;
    bool first_mouse = true;
    float lastFrame = 0.0f; // related with deltaTime

    VortexShader *worldaxis_shader = nullptr;
    unsigned int world_axisVAO;
    unsigned int world_axisVBO;
    bool view_world_axis = false;

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void set_fullscreen();
    void setup_world_axis_buffers();
    void draw_world_axis();
    void draw_world_axis_gizmo();
public:
    glm::vec4 bg_color = glm::vec4(0.4f, 0.45f, 0.5f, 1.0f);
    float deltaTime = 0.0f;
    bool show_wireframe = false;

    VortexWindow(std::string window_name, VortexCamera* camera, int width=800, int height=600);
    ~VortexWindow();

    void run(std::function<void()> draw_callback);
    GLFWwindow* get_window_ptr();
};

