/*
 * File: vortex_obj_reader.hpp
 * Project: VortexEngine
 * Description: Read object files to load 3D models in the scene
 * Author: Mashhood Husnain
 * License: MIT
 */

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
#include "vortex_application.hpp"
#include "vortex_assetmanager.hpp"
#include "util/vortex_behaviour.hpp"
#include "util/vortex_global_vars.hpp"

#include "stb_image.h"

struct SharedMesh;

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct VortexModel_Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};

struct VortexModel_Object
{
    Transform transform;

    std::string name;
    // accessing the object from the overal vertices when loaded from VortexModel class
    int vertex_offset;
    int vertex_count;

    // for object collision
    glm::vec3 b_min;
    glm::vec3 b_max;
    glm::vec3 get_world_min() const {return transform.position + b_min;}
    glm::vec3 get_world_max() const {return transform.position + b_max;};
};

bool check_collision(const VortexModel_Object &a, const VortexModel_Object &b);

class VortexModel
{
public:
    VortexApplication *app;
    std::string model_name;
    
    Transform transform = {
        .position = glm::vec3(0.0f),
        .rotation = glm::vec3(0.0f),
        .scale = glm::vec3(1.0f)
    };

    bool should_destroy = false;
    bool is_active = true;

    std::string file_path;

    SharedMesh *shared_data;

    std::vector<VortexMonoBehaviour*> behaviours;
    std::vector<std::string> script_names;

    VortexModel(const std::string& model_path, VortexApplication *window);
    void draw(const VortexShader& shader, VortexCamera& camera, bool wireframe);
    std::vector<VortexModel_Object>& get_objects();

    void add_behaviour(const std::string &script_name, VortexMonoBehaviour *script);
    void update(float deltaTime);
    void late_update(float deltaTime);
    
    ~VortexModel();
};

void align_on_top(VortexModel& top_obj, const VortexModel& bottom_obj);
