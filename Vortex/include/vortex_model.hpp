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

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_application.hpp"
#include "vortex_assetmanager.hpp"
#include "vortex_behaviour.hpp"
#include "util/vortex_global_vars.hpp"
#include "util/vortex_logs.hpp"
#include "vortex_collider.hpp"

#include "stb_image.h"

struct SharedMesh;

struct Transform
{
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;

    glm::vec3 get_euler() const
    {
        return glm::degrees(glm::eulerAngles(orientation));
    }

    void set_euler(const glm::vec3& euler)
    {
        orientation = glm::quat(glm::radians(euler));
    }

    void set_position(glm::vec3 new_position)
    {
        position = new_position;
    }

    glm::vec3 get_position()
    {
        return position;
    }
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

    VortexBoxCollider collider;
};

class VortexModel
{
    glm::mat4 m_cached_matrix = glm::mat4(1.0f);
    glm::vec3 m_last_pos;
    glm::quat m_last_rot;
    glm::vec3 m_last_scale;

    bool m_is_dirty = true;
public:
    VortexApplication *app;
    std::string model_name;
    
    Transform transform = {
        .position = glm::vec3(0.0f),
        .orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
        .scale = glm::vec3(1.0f)
    };

    bool should_destroy = false;
    bool is_active = true;

    bool show_collider = false;

    bool is_selected = false;

    std::string file_path;

    SharedMesh *shared_data;
    glm::vec3 collider_scale = glm::vec3(1.0f);

    glm::mat4 model_matrix;

    std::vector<VortexMonoBehaviour*> behaviours;
    std::vector<std::string> script_names;

    std::vector<bool> active_parts;

    VortexModel(const std::string &model_path, VortexApplication *window);
    void draw(const VortexShader &shader, VortexCamera &camera, bool wireframe);
    std::vector<VortexModel_Object>& get_objects();

    void add_behaviour(const std::string &script_name, VortexMonoBehaviour *script);
    void update(float deltaTime);
    void late_update(float deltaTime);
    glm::mat4 get_model_matrix();
    void set_model_matrix(glm::mat4 matrix);
    
    ~VortexModel();

    VortexMonoBehaviour* get_behaviour(const std::string& script_name);

    void send_message(const std::string &message, void *data=nullptr);

    template <typename T>
    T* get_componant()
    {
        for (VortexMonoBehaviour *script : behaviours)
        {
            T *target = dynamic_cast<T*>(script);
            if (target) return target;
        }

        return nullptr;
    }
};

void align_on_top(VortexModel& top_obj, const VortexModel& bottom_obj);
