/*
 * File: vortex_shaders.cpp
 * Project: VortexEngine
 * Description: Implementation for reading shaders
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_shaders.hpp"

bool VortexShader::read_shader(const std::string& vertex_path, const std::string& fragment_path)
{
    std::ifstream vertex_file(vertex_path);
    std::ifstream fragment_file(fragment_path);

    if (!vertex_file.is_open())
    {
        std::cerr << "[SHADER ERROR] could not open " << vertex_path << " file"<< std::endl;
        return false;
    }

    if (!fragment_file.is_open())
    {
        std::cerr << "[SHADER ERROR] could not open " << fragment_path <<  " file" << std::endl;
        return false;
    }

    std::stringstream vertex_stream, fragment_stream;

    vertex_stream << vertex_file.rdbuf();
    fragment_stream << fragment_file.rdbuf();

    vertex_file.close();
    fragment_file.close();

    vertex_shader_code = vertex_stream.str();
    fragment_shader_code = fragment_stream.str();

    return true;
}

void VortexShader::init_shader()
{
    const char* vertex_shader_source = vertex_shader_code.c_str();
    const char* fragment_shader_source = fragment_shader_code.c_str();

    // compile
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    // link into program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    // delete shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

VortexShader::VortexShader(const std::string vertex_path, const std::string fragment_path)
{
    if (!read_shader(vertex_path, fragment_path)) exit(EXIT_FAILURE);;
    init_shader();
}

void VortexShader::use()
{
    glUseProgram(shader_program);
}

void VortexShader::setMat4(const std::string &name, const glm::mat4 &mat)
{
    GLint location = glGetUniformLocation(shader_program, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

