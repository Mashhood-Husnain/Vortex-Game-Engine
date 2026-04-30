#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

void VortexProject::save_project(Snapshot *snapshot)
{
    if (!std::filesystem::exists("saves"))
    {
        std::filesystem::create_directory("saves");
    }

    json save_data;
    save_data["project_name"] = snapshot->project_name;

    save_data["models"] = json::array();
    for (VortexModel *model : snapshot->active_models)
    {
        json j_model;
        j_model["path"] = model->file_path;
        j_model["name"] = model->model_name;
        j_model["position"] = {model->transform.position.x, model->transform.position.y, model->transform.position.z};
        j_model["orientation"] = {
            model->transform.orientation.w,
            model->transform.orientation.x, 
            model->transform.orientation.y, 
            model->transform.orientation.z
        };
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
    for (ParticleSystem *ps : snapshot->active_systems)
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

    save_data["m_selected_skybox_idx"] = snapshot->m_selected_skybox_idx;
    save_data["m_selected_shader_idx"] = snapshot->m_selected_shader_idx;

    if (snapshot->window->get_editor_camera())
    {
        save_data["camera"] = json::object();
        save_data["camera"]["position"] = {
            snapshot->window->get_editor_camera()->get_position().x,
            snapshot->window->get_editor_camera()->get_position().y,
            snapshot->window->get_editor_camera()->get_position().z
        };
        save_data["camera"]["yaw"] = snapshot->window->get_editor_camera()->get_yaw();
        save_data["camera"]["pitch"] = snapshot->window->get_editor_camera()->get_pitch();
    }

    std::string file_path = "saves/" + snapshot->project_name + ".vtx";
    std::ofstream file(file_path);

    file << save_data.dump(4);
    file.close();

    VORTEX_INFO("[PROJECT] Successfully saved to ", file_path);
}

std::vector<std::string> VortexProject::search_save_files(const std::string &file_name)
{
    const std::string base_path = "saves/";

    if (!file_name.empty())
    {
        std::string file_path = base_path + file_name + ".vtx";
        if (!std::filesystem::exists(file_path)) return {};

        return {file_path};
    }

    if (!std::filesystem::exists(base_path)) return {};

    std::vector<std::string> save_files;

    for (const auto &entry  : std::filesystem::directory_iterator(base_path))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".vtx")
        {
            save_files.push_back(entry.path().string());
        }
    }

    return save_files;
}

void VortexProject::load_project(Snapshot *snapshot)
{
    std::string file_path = "saves/" + snapshot->project_name + ".vtx";

    if (!std::filesystem::exists(file_path))
    {
        VORTEX_WARN("[PROJECT ERROR] Save file not found: ", file_path);
        return;
    }

    std::ifstream file(file_path);

    json save_data;
    file >> save_data;
    file.close();

    for (VortexModel *model : snapshot->active_models) model->should_destroy = true;
    for (ParticleSystem *ps : snapshot->active_systems) ps->should_destroy = true;

    snapshot->active_models.clear();
    snapshot->active_systems.clear();

    for (const auto &j_model : save_data["models"])
    {
        VortexModel *new_model = new VortexModel(j_model["path"], snapshot->window);

        if (new_model->shared_data)
        {
            for (size_t i = 0; i < new_model->shared_data->objects.size(); i++)
            {
                new_model->active_parts[i] = true;
            }
        }

        new_model->transform.position = glm::vec3(j_model["position"][0], j_model["position"][1], j_model["position"][2]);
        if (j_model.contains("orientation"))
        {
            auto &q = j_model["orientation"];
            new_model->transform.orientation = glm::quat(
                q[0].get<float>(),
                q[1].get<float>(),
                q[2].get<float>(),
                q[3].get<float>()
            );
        }
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

        snapshot->active_models.push_back(new_model);
    }

    for (const auto &j_ps : save_data["particle_systems"])
    {
        ParticleSystem *new_ps = new ParticleSystem(j_ps["max_particles"], snapshot->window, j_ps["name"]);

        auto &emitter = new_ps->get_emitter("Default_Emitter");
        if (j_ps.contains("emitter_pos"))
        {
            emitter.position = glm::vec3(j_ps["emitter_pos"][0], j_ps["emitter_pos"][1], j_ps["emitter_pos"][2]);
        }

        snapshot->active_systems.push_back(new_ps);
    }

    snapshot->m_selected_skybox_idx = save_data["m_selected_skybox_idx"];
    snapshot->m_selected_shader_idx = save_data["m_selected_shader_idx"];

    if (save_data.contains("camera") && snapshot->window->get_editor_camera())
    {
        VortexCamera * editor_cam = snapshot->window->get_editor_camera();
        if (editor_cam)
        {
            glm::vec3 cam_pos = glm::vec3(
                save_data["camera"]["position"][0],
                save_data["camera"]["position"][1],
                save_data["camera"]["position"][2]
            );

            editor_cam->set_rotation(save_data["camera"]["yaw"], save_data["camera"]["pitch"]);

            editor_cam->set_position(cam_pos);
        }
        else
        {
            VORTEX_ERROR("[LOAD ERROR] Failed to load camera data: Editor camera is null!");
        }
    }

    VORTEX_INFO("[PROJECT] Successfully loaded ", snapshot->project_name);
}

bool VortexProject::check_save_state(Snapshot *snapshot)
{
    std::string filepath = "saves/" + snapshot->project_name + ".vtx"; 
    std::ifstream file(filepath);

    if (!file.is_open())
    {
        return snapshot->active_models.empty() && snapshot->active_systems.empty();
    }

    json saved_data;
    try
    {
        file >> saved_data;
    } catch (...)
    {
        return false;
    }

    if (saved_data["m_selected_skybox_idx"] != snapshot->m_selected_skybox_idx) return false;
    if (saved_data["m_selected_shader_idx"] != snapshot->m_selected_shader_idx) return false;

    if (saved_data["models"].size() != snapshot->active_models.size()) return false;
    if (saved_data["particle_systems"].size() != snapshot->active_systems.size()) return false;

    auto float_equals = [](float a, float b) { return std::abs(a - b) < 0.001f; };

    for (size_t i = 0; i < snapshot->active_models.size(); i++)
    {
        VortexModel* model = snapshot->active_models[i];
        json& saved_model = saved_data["models"][i];

        if (saved_model["name"] != model->model_name) return false;

        if (!float_equals(saved_model["position"][0], model->transform.position.x) ||
            !float_equals(saved_model["position"][1], model->transform.position.y) ||
            !float_equals(saved_model["position"][2], model->transform.position.z)) return false;

        if (!float_equals(saved_model["scale"][0], model->transform.scale.x) ||
            !float_equals(saved_model["scale"][1], model->transform.scale.y) ||
            !float_equals(saved_model["scale"][2], model->transform.scale.z)) return false;

        if (!float_equals(saved_model["orientation"][0], model->transform.orientation.w) ||
            !float_equals(saved_model["orientation"][1], model->transform.orientation.x) ||
            !float_equals(saved_model["orientation"][2], model->transform.orientation.y) ||
            !float_equals(saved_model["orientation"][3], model->transform.orientation.z)) return false;

        if (saved_model.contains("show_collider") && saved_model["show_collider"] != model->show_collider) return false;
        
        if (saved_model.contains("collider_scale"))
        {
            if (!float_equals(saved_model["collider_scale"][0], model->collider_scale.x) ||
                !float_equals(saved_model["collider_scale"][1], model->collider_scale.y) ||
                !float_equals(saved_model["collider_scale"][2], model->collider_scale.z)) return false;
        }

        if (saved_model.contains("scripts") && saved_model["scripts"].size() != model->script_names.size()) return false;
    
        if (saved_model.contains("script_data"))
        {
            for (size_t s = 0; s < model->script_names.size(); s++)
            {
                std::string s_name = model->script_names[s];
                
                json live_script_data = json::object();
                model->behaviours[s]->serialize(live_script_data);
                
                if (saved_model["script_data"].contains(s_name))
                {
                    if (saved_model["script_data"][s_name] != live_script_data) return false;
                }
                else if (!live_script_data.empty())
                {
                    return false;
                }
            }
        }
    }

    for (size_t i = 0; i < snapshot->active_systems.size(); i++)
    {
        ParticleSystem* ps = snapshot->active_systems[i];
        json& saved_ps = saved_data["particle_systems"][i];

        if (saved_ps["name"] != ps->name) return false;
        if (saved_ps["max_particles"] != ps->max_particles) return false;
    }

    return true; 
}

void VortexProject::take_snapshot(SnapshotState state, VortexApplication *window, std::string project_name)
{
    if (project_name.empty()) project_name = std::string(VortexGUI::save_project_name);

    Snapshot snapshot = {
        project_name,
        VortexObjectManager::active_models,
        VortexObjectManager::active_particlesystems,
        VortexGUI::m_selected_skybox_idx,
        VortexGUI::m_selected_shader_idx,
        window
    };

    if (state == SnapshotState::SAVE) VortexProject::save_project(&snapshot);
    else if (state == SnapshotState::LOAD) VortexProject::load_project(&snapshot);
}
