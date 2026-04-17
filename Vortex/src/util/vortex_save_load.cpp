#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

void VortexProject::save_project(SaveScene_snapshot *scene_snapshot)
{
    if (!std::filesystem::exists("saves"))
    {
        std::filesystem::create_directory("saves");
    }

    json save_data;
    save_data["project_name"] = scene_snapshot->project_name;

    save_data["models"] = json::array();
    for (VortexModel *model : scene_snapshot->active_models)
    {
        json j_model;
        j_model["path"] = model->file_path;
        j_model["name"] = model->model_name;
        j_model["position"] = {model->transform.position.x, model->transform.position.y, model->transform.position.z};
        j_model["rotation"] = {model->transform.rotation.x, model->transform.rotation.y, model->transform.rotation.z};
        j_model["scale"] = {model->transform.scale.x, model->transform.scale.y, model->transform.scale.z};

        j_model["show_collider"] = model->show_collider;
        j_model["collider_scale"] = {model->collider_scale.x, model->collider_scale.y, model->collider_scale.z};

        j_model["scripts"] = json::array();
        j_model["script_data"] = json::object();
        for (size_t i = 0; i < model->script_names.size(); i++)
        {
            std::string s_name = model->script_names[i];
            j_model["scripts"].push_back(s_name);

            json script_json;
            model->behaviours[i]->serialize(script_json);
            if (!script_json.empty())
            {
                j_model["script_data"][s_name] = script_json;
            }
        }

        save_data["models"].push_back(j_model);
    }

    save_data["particle_systems"] = json::array();
    for (ParticleSystem *ps : scene_snapshot->active_systems)
    {
        json j_ps;
        j_ps["name"] = ps->name;
        j_ps["max_particles"] = ps->max_particles;

        if (!ps->emitter_registry.empty())
        {
            auto &emitter = ps->emitter_registry["Default_Emitter"];
            j_ps["emitter_pos"] = {emitter.position.x, emitter.position.y, emitter.position.z};
        }

        save_data["particle_systems"].push_back(j_ps);
    }

    save_data["m_selected_skybox_idx"] = scene_snapshot->m_selected_skybox_idx;
    save_data["m_selected_shader_idx"] = scene_snapshot->m_selected_shader_idx;

    std::string file_path = "saves/" + scene_snapshot->project_name + ".vtx";
    std::ofstream file(file_path);

    file << save_data.dump(4);
    file.close();

    VORTEX_INFO("[PROJECT] Successfully saved to ", file_path);
}

void VortexProject::load_project(SaveScene_snapshot *scene_snapshot)
{
    std::string file_path = "saves/" + scene_snapshot->project_name + ".vtx";

    if (!std::filesystem::exists(file_path))
    {
        VORTEX_WARN("[PROJECT ERROR] Save file not found: ", file_path);
        return;
    }

    std::ifstream file(file_path);

    json save_data;
    file >> save_data;
    file.close();

    for (VortexModel *model : scene_snapshot->active_models) model->should_destroy = true;
    for (ParticleSystem *ps : scene_snapshot->active_systems) ps->should_destroy = true;

    scene_snapshot->active_models.clear();
    scene_snapshot->active_systems.clear();

    for (const auto &j_model : save_data["models"])
    {
        VortexModel *new_model = new VortexModel(j_model["path"], scene_snapshot->window);

        if (new_model->shared_data)
        {
            for (size_t i = 0; i < new_model->shared_data->objects.size(); i++)
            {
                new_model->active_parts[i] = true;
            }
        }

        new_model->transform.position = glm::vec3(j_model["position"][0], j_model["position"][1], j_model["position"][2]);
        new_model->transform.rotation = glm::vec3(j_model["rotation"][0], j_model["rotation"][1], j_model["rotation"][2]);
        new_model->transform.scale = glm::vec3(j_model["scale"][0], j_model["scale"][1], j_model["scale"][2]);
        new_model->model_name = j_model["name"];

        if (j_model.contains("show_collider"))
        {
            new_model->show_collider = j_model["show_collider"];
        }
        
        if (j_model.contains("collider_scale"))
        {
            new_model->collider_scale = glm::vec3(j_model["collider_scale"][0], j_model["collider_scale"][1], j_model["collider_scale"][2]);
        }

        if (j_model.contains("scripts"))
        {
            for (const auto &j_script_name : j_model["scripts"])
            {
                std::string name = j_script_name;
                VortexMonoBehaviour *new_script = ScriptRegistry::get().create(name);

                if (new_script)
                {
                    if (j_model.contains("script_data") && j_model["script_data"].contains(name))
                    {
                        new_script->deserialize(j_model["script_data"][name]);
                    }

                    new_model->add_behaviour(name, new_script);
                }
            }
        }

        scene_snapshot->active_models.push_back(new_model);
    }

    for (const auto &j_ps : save_data["particle_systems"])
    {
        ParticleSystem *new_ps = new ParticleSystem(j_ps["max_particles"], scene_snapshot->window, j_ps["name"]);

        auto &emitter = new_ps->get_emitter("Default_Emitter");
        if (j_ps.contains("emitter_pos"))
        {
            emitter.position = glm::vec3(j_ps["emitter_pos"][0], j_ps["emitter_pos"][1], j_ps["emitter_pos"][2]);
        }

        scene_snapshot->active_systems.push_back(new_ps);
    }

    scene_snapshot->m_selected_skybox_idx = save_data["m_selected_skybox_idx"];
    scene_snapshot->m_selected_shader_idx = save_data["m_selected_shader_idx"];

    VORTEX_INFO("[PROJECT] Successfully loaded ", scene_snapshot->project_name);
}
