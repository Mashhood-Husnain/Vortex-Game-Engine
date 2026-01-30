#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>

class OpenGLWindow
{
    GLFWwindow *window = nullptr;
    int default_window_width;
    int default_window_height;
    std::string window_name;
    glm::vec4 bg_color = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void check_input();
public:
    OpenGLWindow(std::string window_name, int width=800, int height=600);
    ~OpenGLWindow();

    void run();

};

