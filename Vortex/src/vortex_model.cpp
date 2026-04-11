/*
 * File: vortex_model.cpp
 * Project: VortexEngine
 * Description: Read object files to load 3D models in the scene
 * Author: Mashhood Husnain
 * License: MIT
 */


#include "vortex_model.hpp"

VortexModel::VortexModel(const std::string& path, VortexApplication *window)
{
    if (!window)
    {
        std::cout << "[MODEL ERROR] 'window' for model not specified" << std::endl;
        exit(EXIT_FAILURE);
    }

    app = window;
    file_path = path;

    shared_data = VortexAssetManager::get_mesh(path);
}

void VortexModel::draw(const VortexShader& shader, VortexCamera& camera, bool wireframe)
{
    const VortexShader *active_shader = &shader;

    if (app->shadow_manager->is_active)
    {
        active_shader = app->shadow_manager->shadow_shader;
    }

    active_shader->use();

    if (app->shadow_manager->is_active)
    {
        active_shader->setMat4("lightSpaceMatrix", app->shadow_manager->light_space_matrix);
    }
    else
    {
        active_shader->setMat4("view", camera.getViewMatrix());
        active_shader->setMat4("projection", camera.getProjectionMatrix());

        active_shader->setVec3("lightPos", GLOBAL::LIGHTPOS);
        active_shader->setVec3("viewPos", camera.position);

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
        glBindTexture(GL_TEXTURE_2D, app->shadow_manager->shadow_map);
        active_shader->setInt("shadowMap", 3);
        active_shader->setMat4("lightSpaceMatrix", app->shadow_manager->light_space_matrix);

        app->gui.inspector_info(this, nullptr);
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
    for (const auto& obj : shared_data->objects)
    {
        if (obj.vertex_count == 0) continue;

        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, this->position + obj.position);

        glm::vec3 total_rot = this->rotation + obj.rotation;
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.x), glm::vec3(1, 0, 0));
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.y), glm::vec3(0, 1, 0));
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.z), glm::vec3(0, 0, 1));
        model_matrix = glm::scale(model_matrix, glm::vec3(this->scale * obj.scale));

        active_shader->setMat4("model", model_matrix);

        glDrawArrays(GL_TRIANGLES, obj.vertex_offset, obj.vertex_count);
    }

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

std::vector<VortexModel_Object>& VortexModel::get_objects()
{
    return shared_data->objects;
}

bool check_collision(const VortexModel_Object &a, const VortexModel_Object &b)
{
    bool x_axis = a.get_world_min().x <= b.get_world_max().x && a.get_world_max().x >= b.get_world_min().x;
    bool y_axis = a.get_world_min().y <= b.get_world_max().y && a.get_world_max().y >= b.get_world_min().y;
    bool z_axis = a.get_world_min().z <= b.get_world_max().z && a.get_world_max().z >= b.get_world_min().z;

    return x_axis && y_axis && z_axis;
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
    script->gameObject = this;
    behaviours.push_back(script);
    script_names.push_back(script_name);
}

void VortexModel::update(float deltaTime)
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->on_update(deltaTime);
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
    float bottom_surface = bottom_obj.position.y + (bottom_obj.shared_data->model_height * bottom_obj.scale.y / 2.0f);
    float top_half_height = (top_obj.shared_data->model_height * top_obj.scale.y) / 2.0f;
    
    top_obj.position.y = bottom_surface + top_half_height;
}
