#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <functional>

#include "opengl_camera.hpp"

class OpenGLWindow
{
    GLFWwindow *window = nullptr;
    int default_window_width = 800;
    int default_window_height = 600;
    std::string window_name;
    glm::vec4 bg_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    bool is_fullscreen = false;
    int stored_window_x_pos;
    int stored_window_y_pos;
    int stored_window_width;
    int stored_window_height;
    OpenGLCamera *camera = nullptr;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float mouse_last_x = default_window_width / 2;
    float mouse_last_y = default_window_height / 2;
    bool first_mouse = true;

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void set_fullscreen();
    void check_camera_movement();
public:

    OpenGLWindow(std::string window_name, OpenGLCamera* camera, int width=800, int height=600);
    ~OpenGLWindow();

    void run(std::function<void()> draw_callback);

};

