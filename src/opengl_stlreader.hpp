#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

class STLModel
{
    void load_stl(const std::string& path);
    void load_ascii(const std::string& path);
    void load_binary(const std::string& path);
    void setup_mesh();
public:
    unsigned VAO, VBO;
    std::vector<Vertex> vertices;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    float scale = 1.0f;

    STLModel(const std::string& model_path);
    void draw(const openGLShader& shader, OpenGLCamera& camera);
};
