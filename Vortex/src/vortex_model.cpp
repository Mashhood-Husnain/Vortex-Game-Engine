/*
 * File: vortex_model.cpp
 * Project: VortexEngine
 * Description: Read object files to load 3D models in the scene
 * Author: Mashhood Husnain
 * License: MIT
 */


#include "vortex_model.hpp"
#include "vortex_camera.hpp"
#include "vortex_application.hpp"
#include "vortex_shaders.hpp"

VortexModel::VortexModel(const std::string& path, VortexApplication *window)
{
    if (!window)
    {
        VORTEX_ERROR("[MODEL ERROR] 'window' for model not specified");
        exit(EXIT_FAILURE);
    }

    app = window;
    file_path = path;

    std::string base_name = std::filesystem::path(path).stem().string();
    int id = VortexAssetManager::spawn_counts[base_name]++;

    model_name = base_name + "_" + std::to_string(id);

    shared_data = VortexAssetManager::get_mesh(path);
    if (shared_data)
    {
        active_parts.resize(shared_data->objects.size(), true);
    }
}

glm::mat4 VortexModel::get_model_matrix()
{
    if (transform.position != m_last_pos || 
        transform.orientation != m_last_rot || 
        transform.scale    != m_last_scale || 
        m_is_dirty) 
    {
        m_cached_matrix = glm::mat4(1.0f);
        m_cached_matrix = glm::translate(m_cached_matrix, transform.position);

        m_cached_matrix *= glm::mat4_cast(transform.orientation);

        m_cached_matrix = glm::scale(m_cached_matrix, transform.scale);

        m_last_pos   = transform.position;
        m_last_rot   = transform.orientation;
        m_last_scale = transform.scale;
        
        m_is_dirty = false;
    }

    return m_cached_matrix;
}

void VortexModel::set_model_matrix(glm::mat4 matrix)
{
    model_matrix = matrix;
}

void VortexModel::draw(const VortexShader &shader, VortexCamera &camera, bool wireframe)
{
    const VortexShader *active_shader = &shader;

    if (app->get_shadow_manager()->is_active)
    {
        active_shader = app->get_shadow_manager()->shadow_shader;
    }

    active_shader->use();

    if (app->get_shadow_manager()->is_active)
    {
        active_shader->setMat4("lightSpaceMatrix", app->get_shadow_manager()->light_space_matrix);
    }
    else
    {
        active_shader->setMat4("view", camera.getViewMatrix());
        active_shader->setMat4("projection", camera.getProjectionMatrix());

        active_shader->setVec3("lightPos", GLOBAL::LIGHTPOS);
        active_shader->setVec3("viewPos", camera.get_position());

        // textures (multi-sampling)
        active_shader->setInt("u_hasTexture", shared_data->texture_id!= 0);
        active_shader->setInt("u_hasRoughness", shared_data->roughness_id != 0);
        active_shader->setInt("u_hasMetallic", shared_data->metallic_id != 0);

        // 0 - diffuse
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shared_data->texture_id); // If 0, it unbinds
        active_shader->setInt("u_diffuseMap", 0);

        // 1 - roughness
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shared_data->roughness_id);
        active_shader->setInt("u_roughnessMap", 1);

        // 2 - metallic
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shared_data->metallic_id);
        active_shader->setInt("u_metallicMap", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, app->get_shadow_manager()->shadow_map);
        active_shader->setInt("shadowMap", 3);
        active_shader->setMat4("lightSpaceMatrix", app->get_shadow_manager()->light_space_matrix);
    }

    // rendering wireframe
    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
    } else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(shared_data->VAO);

    glm::mat4 base_matrix = get_model_matrix();
    for (size_t i = 0; i < shared_data->objects.size(); i++)
    {
        if (!active_parts[i]) continue;

        VortexModel_Object &obj = shared_data->objects[i];

        if (obj.vertex_count == 0) continue;

        glm::mat4 local_model_matrix = glm::translate(base_matrix, obj.transform.position);
        local_model_matrix *= glm::mat4_cast(obj.transform.orientation);        
        local_model_matrix = glm::scale(local_model_matrix, obj.transform.scale);

        active_shader->setMat4("model", local_model_matrix);

        glDrawArrays(GL_TRIANGLES, obj.vertex_offset, obj.vertex_count);
    }

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

std::vector<VortexModel_Object>& VortexModel::get_objects()
{
    return shared_data->objects;
}

VortexModel::~VortexModel()
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        delete script;
    }
    behaviours.clear();
    app = nullptr;
    shared_data = nullptr;
}

void VortexModel::add_behaviour(const std::string &script_name, VortexMonoBehaviour *script)
{
    script->vortexGameObject = this;
    script->vortexEngine = app;
    script->vortexTransform = &transform;
    behaviours.push_back(script);
    script_names.push_back(script_name);
}

void VortexModel::update(float deltaTime)
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->on_update(deltaTime);
    }

    if (rigidbody)
    {
        rigidbody->on_update(deltaTime);
    }
}

void VortexModel::late_update(float deltaTime)
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->late_update(deltaTime);
    }
}

void align_on_top(VortexModel& top_obj, const VortexModel& bottom_obj)
{
    float bottom_surface = bottom_obj.transform.position.y + (bottom_obj.shared_data->model_height * bottom_obj.transform.scale.y / 2.0f);
    float top_half_height = (top_obj.shared_data->model_height * top_obj.transform.scale.y) / 2.0f;
    
    top_obj.transform.position.y = bottom_surface + top_half_height;
}

VortexMonoBehaviour* VortexModel::get_behaviour(const std::string& script_name)
{
    for (size_t i = 0; i < script_names.size(); i++)
    {
        if (script_names[i] == script_name)
        {
            return behaviours[i];
        }
    }
    return nullptr;
}

void VortexModel::send_message(const std::string &message, void *data)
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->on_message(message, data);
    }
}

glm::vec3 VortexModel::get_world_bounds_min()
{
    glm::vec3 local_min = shared_data->collider.min * collider_scale;
    glm::vec3 local_max = shared_data->collider.max * collider_scale;

    glm::mat4 model_matrix = get_model_matrix();

    glm::vec3 corners[8] = {
        glm::vec3(local_min.x, local_min.y, local_min.z),
        glm::vec3(local_max.x, local_min.y, local_min.z),
        glm::vec3(local_min.x, local_max.y, local_min.z),
        glm::vec3(local_max.x, local_max.y, local_min.z),
        glm::vec3(local_min.x, local_min.y, local_max.z),
        glm::vec3(local_max.x, local_min.y, local_max.z),
        glm::vec3(local_min.x, local_max.y, local_max.z),
        glm::vec3(local_max.x, local_max.y, local_max.z)
    };

    glm::vec3 world_min = glm::vec3(1e10f);

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed = glm::vec3(model_matrix * glm::vec4(corners[i], 1.0f));
        world_min = glm::min(world_min, transformed);
    }

    return world_min;
}

glm::vec3 VortexModel::get_world_bounds_max()
{
    glm::vec3 local_min = shared_data->collider.min * collider_scale;
    glm::vec3 local_max = shared_data->collider.max * collider_scale;
    glm::mat4 model_matrix = get_model_matrix();

    glm::vec3 corners[8] = {
        glm::vec3(local_min.x, local_min.y, local_min.z),
        glm::vec3(local_max.x, local_min.y, local_min.z),
        glm::vec3(local_min.x, local_max.y, local_min.z),
        glm::vec3(local_max.x, local_max.y, local_min.z),
        glm::vec3(local_min.x, local_min.y, local_max.z),
        glm::vec3(local_max.x, local_min.y, local_max.z),
        glm::vec3(local_min.x, local_max.y, local_max.z),
        glm::vec3(local_max.x, local_max.y, local_max.z)
    };

    glm::vec3 world_max = glm::vec3(-1e10f);
    
    for (int i = 0; i < 8; ++i)
    {
        glm::vec3 transformed = glm::vec3(model_matrix * glm::vec4(corners[i], 1.0f));
        world_max = glm::max(world_max, transformed);
    }

    return world_max;
}

std::vector<glm::vec3> VortexModel::get_world_bounds_min_max()
{
    glm::vec3 local_min = shared_data->collider.min * collider_scale;
    glm::vec3 local_max = shared_data->collider.max * collider_scale;
    glm::mat4 matrix = get_model_matrix();

    glm::vec3 corners[8] = {
        glm::vec3(local_min.x, local_min.y, local_min.z),
        glm::vec3(local_max.x, local_min.y, local_min.z),
        glm::vec3(local_min.x, local_max.y, local_min.z),
        glm::vec3(local_max.x, local_max.y, local_min.z),
        glm::vec3(local_min.x, local_min.y, local_max.z),
        glm::vec3(local_max.x, local_min.y, local_max.z),
        glm::vec3(local_min.x, local_max.y, local_max.z),
        glm::vec3(local_max.x, local_max.y, local_max.z)
    };

    glm::vec3 world_min = glm::vec3(1e10f);
    glm::vec3 world_max = glm::vec3(-1e10f);

    for (int i = 0; i < 8; i++)
    {
        glm::vec3 transformed = glm::vec3(model_matrix * glm::vec4(corners[i], 1.0f));
        world_min = glm::min(world_min, transformed);
        world_max = glm::max(world_max, transformed);
    }

    std::vector<glm::vec3> min_max = {
        world_min,
        world_max
    };

    return min_max;
}
