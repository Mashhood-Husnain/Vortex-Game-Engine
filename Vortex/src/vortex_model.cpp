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

void VortexModel::bind_lights_to_shader(VortexShader* shader, bool use_scene_lights)
{
    const int MAX_LIGHTS = 16;
    int light_count = 0;
    bool using_safety_net = false;

    glm::vec3 l_positions[MAX_LIGHTS];
    glm::vec3 l_colors[MAX_LIGHTS];
    float l_ambients[MAX_LIGHTS];

    if (use_scene_lights)
    {
        // 1. RENDER VIEW / PLAY MODE LOGIC
        for (VortexModel *scene_model : VortexObjectManager::active_models)
        {
            if (scene_model->is_active && scene_model->light)
            {
                l_positions[light_count] = scene_model->transform.position;
                l_colors[light_count] = scene_model->light->color * scene_model->light->intensity;
                l_ambients[light_count] = scene_model->light->ambient_strength;

                light_count++;
                if (light_count >= MAX_LIGHTS) break;
            }
        }
    }
    else
    {
        l_positions[0] = glm::vec3(10.0f, 50.0f, 20.0f);
        l_colors[0] = glm::vec3(1.0f);
        l_ambients[0] = 0.5f;

        light_count = 1;
        using_safety_net = true;
    }

    shader->setInt("numLights", light_count);
    shader->setFloat("constantFalloff", 1.0f);
    shader->setFloat("linearFalloff", using_safety_net ? 0.0f : 0.09f);
    shader->setFloat("quadraticFalloff", using_safety_net ? 0.0f : 0.032f);

    for (int i = 0; i < light_count; i++)
    {
        std::string pos_name = "lightPos[" + std::to_string(i) + "]";
        std::string col_name = "lightColor[" + std::to_string(i) + "]";
        std::string amb_name = "ambientStrength[" + std::to_string(i) + "]";

        shader->setVec3(pos_name, l_positions[i]);
        shader->setVec3(col_name, l_colors[i]);
        shader->setFloat(amb_name, l_ambients[i]);
    }
}

VortexModel::VortexModel(const std::string& path, VortexApplication *window, const std::string& loaded_name)
{
    if (!window)
    {
        VORTEX_ERROR("[MODEL ERROR] 'window' for model not specified");
        exit(EXIT_FAILURE);
    }

    app = window;
    file_path = path;

    std::string base_name = std::filesystem::path(path).stem().string();
    if (loaded_name.empty())
    {
        int id = VortexAssetManager::spawn_counts[base_name]++;
        model_name = base_name + "_" + std::to_string(id);
    }
    else
    {
        model_name = loaded_name;

        size_t last_underscore = model_name.find_last_of('_');
        if (last_underscore != std::string::npos)
        {
            try
            {
                int parsed_id = std::stoi(model_name.substr(last_underscore + 1));
                if (parsed_id >= VortexAssetManager::spawn_counts[base_name])
                {
                    VortexAssetManager::spawn_counts[base_name] = parsed_id + 1;
                }
            }
            catch (...) {}
        }
    }

    glGenQueries(1, &occlusion_query);

    shared_data = VortexAssetManager::get_mesh(path);
    if (shared_data)
    {
        active_parts.resize(shared_data->objects.size(), true);
        texture_id = shared_data->texture_id;
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

void VortexModel::draw(const VortexShader &shader, VortexCamera &camera, bool wireframe, bool use_scene_lights)
{
    VortexShader *active_shader = const_cast<VortexShader*>(&shader);

    // if (app->get_shadow_manager()->is_active)
    // {
    //     active_shader = app->get_shadow_manager()->shadow_shader;
    // }

    active_shader->use();

    if (/*app->get_shadow_manager()->is_active*/ false)
    {
        // active_shader->setMat4("lightSpaceMatrix", app->get_shadow_manager()->light_space_matrix);
    }
    else
    {
        active_shader->setMat4("view", camera.getViewMatrix());
        active_shader->setMat4("projection", camera.getProjectionMatrix());
        active_shader->setVec3("viewPos", camera.get_position());

        bind_lights_to_shader(active_shader, use_scene_lights);

        // textures (multi-sampling)
        active_shader->setInt("u_hasTexture", texture_id != 0);
        active_shader->setInt("u_hasRoughness", shared_data->roughness_id != 0);
        active_shader->setInt("u_hasMetallic", shared_data->metallic_id != 0);
        active_shader->setVec2("u_textureScale", texture_scale);

        // 0 - diffuse
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id); // If 0, it unbinds
        active_shader->setInt("u_diffuseMap", 0);

        // 1 - roughness
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shared_data->roughness_id);
        active_shader->setInt("u_roughnessMap", 1);

        // 2 - metallic
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shared_data->metallic_id);
        active_shader->setInt("u_metallicMap", 2);

        // glActiveTexture(GL_TEXTURE3);
        // glBindTexture(GL_TEXTURE_2D, app->get_shadow_manager()->shadow_map);
        // active_shader->setInt("shadowMap", 3);
        // active_shader->setMat4("lightSpaceMatrix", app->get_shadow_manager()->light_space_matrix);
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

    delete rigidbody;
    delete light;

    rigidbody = nullptr;
    light = nullptr;

    for (VortexDecal* d : decals) delete d;
    decals.clear();

    glDeleteQueries(1, &occlusion_query);

    VortexAssetManager::release_texture(texture_path);
    VortexAssetManager::release_mesh(file_path);

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

void VortexModel::update()
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->on_update();
    }

    if (rigidbody)
    {
        rigidbody->on_update();
    }

    for (VortexDecal* decal : decals) decal->on_update();
}

void VortexModel::late_update()
{
    for (VortexMonoBehaviour *script : behaviours)
    {
        script->late_update();
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
        glm::vec3 transformed = glm::vec3(matrix * glm::vec4(corners[i], 1.0f));
        world_min = glm::min(world_min, transformed);
        world_max = glm::max(world_max, transformed);
    }

    std::vector<glm::vec3> min_max = {
        world_min,
        world_max
    };

    return min_max;
}

VortexModel* VortexModel::clone()
{
    VortexModel* new_model = new VortexModel(file_path, app);

    new_model->transform = transform;
    new_model->show_collider = show_collider;
    new_model->collider_scale = collider_scale;
    new_model->active_parts = active_parts;
    new_model->folder = folder;
    new_model->texture_id = texture_id;
    new_model->texture_path = texture_path;
    new_model->texture_scale = texture_scale;

    new_model->transform.position.x += 1.0f;

    if (rigidbody)
    {
        new_model->rigidbody = new VortexRigidbody();
        new_model->rigidbody->vortexGameObject = new_model;
        new_model->rigidbody->vortexTransform = &new_model->transform;

        json rb_data;
        rigidbody->serialize(rb_data);
        new_model->rigidbody->deserialize(rb_data);
    }

    if (light)
    {
        new_model->light = new VortexLight();
        new_model->light->vortexGameObject = new_model;
        new_model->light->vortexTransform = &new_model->transform;

        json l_data;
        light->serialize(l_data);
        new_model->light->deserialize(l_data);
    }

    for (VortexDecal* d : decals)
    {
        VortexDecal* new_decal = new VortexDecal();
        new_decal->vortexGameObject = new_model;
        new_decal->vortexEngine = app;
        new_decal->vortexTransform = &new_model->transform;

        new_decal->on_start();

        json d_data;
        d->serialize(d_data);
        new_decal->deserialize(d_data);

        new_model->decals.push_back(new_decal);
    }

    for (size_t i = 0; i < script_names.size(); i++)
    {
        std::string script_name = script_names[i];
        VortexMonoBehaviour* original_script = behaviours[i];

        VortexMonoBehaviour* new_script = ScriptRegistry::get().create(script_name);
        if (new_script)
        {
            json script_data;
            original_script->serialize(script_data);
            new_script->deserialize(script_data);

            new_model->add_behaviour(script_name, new_script);
        }
    }

    return new_model;
}
