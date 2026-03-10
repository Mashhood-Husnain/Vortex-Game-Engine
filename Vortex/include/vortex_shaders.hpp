/*
 * File: vortex_shaders.hpp
 * Project: VortexEngine
 * Description: Implementation for reading shaders
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

class VortexShader
{
    std::string vertex_shader_code;
    std::string fragment_shader_code;

    bool read_shader(const std::string& vertex_path, const std::string& fragment_path);
    void init_shader();
public:
    unsigned int shader_program;

    VortexShader(const std::string vertex_path, const std::string fragment_path);
    void use();
    void setMat4(const std::string &name, const glm::mat4 &mat4);
};
