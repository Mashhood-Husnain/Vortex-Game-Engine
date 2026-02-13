#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class OpenGLShader
{
    std::string vertex_shader_code;
    std::string fragment_shader_code;

    bool read_shader(std::string& vertex_path, std::string& fragment_path);
    void init_shader();
public:
    unsigned int shader_program;

    OpenGLShader(std::string vertex_path, std::string fragment_path);
};
