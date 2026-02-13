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
    glm::vec2 tex_coords;
};

class OpenGLModel
{
    unsigned int VAO, VBO;
    std::vector<Vertex> vertices;
    void load_obj(const std::string& path);
    void setup_mesh();
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    float scale = 1.0f;
    unsigned int texture_id = 0;

    OpenGLModel(const std::string& model_path);
    void draw(const OpenGLShader& shader, OpenGLCamera& camera, bool wireframe);
};

