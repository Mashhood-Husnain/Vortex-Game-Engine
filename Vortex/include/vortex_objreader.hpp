#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <filesystem>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "stb_image.h"

struct VortexModel_Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct VortexModel_Object
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation;
    glm::vec3 scale;

    std::string name;
    // accessing the object from the overal vertices when loaded from VortexModel class
    int vertex_offset;
    int vertex_count;

    // for object collision
    glm::vec3 b_min;
    glm::vec3 b_max;
    glm::vec3 get_world_min() const {return position + b_min;}
    glm::vec3 get_world_max() const {return position + b_max;};
};

bool check_collision(const VortexModel_Object &a, const VortexModel_Object &b);

class VortexModel
{
    unsigned int VAO=0;
    unsigned int VBO=0;
    float min_y=0.0f;
    float max_y=0.0f;
    std::string model_name;
    std::vector<VortexModel_Vertex> vertices;
    std::vector<VortexModel_Object> objects;
    void load_obj(const std::string& path);
    void parse_mtl(const std::string& mtl_filename);
    unsigned int load_texture(const std::string& texture_path);
    void setup_mesh();
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    unsigned int texture_id=0;
    unsigned int roughness_id=0;
    unsigned int metallic_id=0;
    unsigned int normal_id=0;
    float model_height=0.0f;

    VortexModel(const std::string& model_path);
    void draw(const VortexShader& shader, VortexCamera& camera, bool wireframe);
    std::vector<VortexModel_Object>& get_objects();
    ~VortexModel();
};
