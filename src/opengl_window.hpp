#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <functional>

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

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void set_fullscreen();
public:
    OpenGLWindow(std::string window_name, int width=800, int height=600);
    ~OpenGLWindow();

    void run(std::function<void()> draw_callback);

};

