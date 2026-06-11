#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

void VortexProject::save_project(Snapshot *snapshot)
{
    std::string prefix = "temp_playmode_backup_";
    std::string folder_name = snapshot->project_name;
    if (folder_name.rfind(prefix, 0) == 0)
    {
        folder_name = folder_name.substr(prefix.length());
    }

    if (!std::filesystem::exists(SAVE_DIRECTORY))
    {
        std::filesystem::create_directory(SAVE_DIRECTORY);
    }

    std::string project_save_dir = SAVE_DIRECTORY + "/" + folder_name;
    if (!std::filesystem::exists(project_save_dir))
    {
        std::filesystem::create_directory(project_save_dir);
    }

    std::string project_asset_dir = project_save_dir + "/" + ASSET_DIR_SCRIPTS;
    if (!std::filesystem::exists(project_asset_dir))
    {
        std::filesystem::create_directory(project_asset_dir);
    }

    std::string project_audio_dir = project_save_dir + "/" + ASSET_DIR_AUDIO;
    if (!std::filesystem::exists(project_audio_dir))
    {
        std::filesystem::create_directory(project_audio_dir);
    }

    std::string project_models_dir = project_save_dir + "/" + ASSET_DIR_MODELS;
    if (!std::filesystem::exists(project_models_dir))
    {
        std::filesystem::create_directory(project_models_dir);
    }

    std::string project_models_obj_dir = project_models_dir + "/" + ASSET_DIR_MODELS_OBJ;
    if (!std::filesystem::exists(project_models_obj_dir))
    {
        std::filesystem::create_directory(project_models_obj_dir);
    }

    std::string project_models_mtl_dir = project_models_dir + "/" + ASSET_DIR_MODELS_MTL;
    if (!std::filesystem::exists(project_models_mtl_dir))
    {
        std::filesystem::create_directory(project_models_mtl_dir);
    }

    std::string project_models_mtl_textures_dir = project_models_mtl_dir + "/" + ASSET_DIR_MODELS_MTL_TEXTURES;
    if (!std::filesystem::exists(project_models_mtl_textures_dir))
    {
        std::filesystem::create_directory(project_models_mtl_textures_dir);
    }

    json save_data;

    save_data["project_name"] = snapshot->project_name;
    save_data["models"] = json::array();
    save_data["empty_folders"] = snapshot->explicit_empty_folders;

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

        if (model->rigidbody)
        {
            json rb_data;
            model->rigidbody->serialize(rb_data);
            j_model["rigidbody"] = rb_data;
        }

        if (model->light)
        {
            json l_data;
            model->light->serialize(l_data);
            j_model["light"] = l_data;
        }

        j_model["folder"] = model->folder;

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

    std::string file_path = project_save_dir + "/" + snapshot->project_name + ".vtx";

    std::string json_string = save_data.dump(4);
    VortexEncrypt::write_encrypted(file_path, json_string);

    if (snapshot->settings_snapshot) save_project_settings(snapshot->settings_snapshot, snapshot->project_name);

    VORTEX_INFO("[PROJECT] Successfully saved to: ", file_path);
}

void VortexProject::save_project_settings(SettingsSnapshot *settings_snapshot, const std::string project_name)
{
    if (!settings_snapshot) return;
    if (project_name.empty() || project_name.find("temp_playmode_backup_") == 0) return;

    std::string path = SAVE_DIRECTORY + "/" + project_name + "/" + project_name + "_settings.json";

    nlohmann::json j;
    j["preferred_ide_path"] = std::string(settings_snapshot->preferred_ide_path);

    std::ofstream file(path);
    if (file.is_open())
    {
        file << j.dump(4);
        file.close();
        VORTEX_INFO("[SETTINGS] Saved project settings: ", path);
    }
}

std::vector<std::string> VortexProject::get_project_names()
{
    const std::string base_path = SAVE_DIRECTORY + "/";
    const std::string prefix = "temp_playmode_backup_";

    if (!std::filesystem::exists(base_path)) return {};

    std::vector<std::string> project_names;

    for (const auto& entry : std::filesystem::directory_iterator(base_path))
    {
        if (!entry.is_directory()) continue;

        std::string folder_name = entry.path().filename().string();

        if (folder_name.rfind(prefix, 0) == 0) continue;

        std::filesystem::path project_file = entry.path() / (folder_name + ".vtx");

        if (std::filesystem::exists(project_file))
        {
            project_names.push_back(folder_name);
        }
    }

    return project_names;
}

void VortexProject::load_project(Snapshot *snapshot)
{
    std::string prefix = "temp_playmode_backup_";
    std::string folder_name = snapshot->project_name;
    if (folder_name.rfind(prefix, 0) == 0)
    {
        folder_name = folder_name.substr(prefix.length());
    }

    std::string file_path = SAVE_DIRECTORY + "/" + folder_name + "/" + snapshot->project_name  + ".vtx";
    if (!std::filesystem::exists(file_path))
    {
        VORTEX_WARN("[PROJECT ERROR] Save file not found: ", file_path);
        return;
    }

    std::string decrypted_data = VortexEncrypt::read_decrypted(file_path);
    if (decrypted_data.empty()) return;

    json save_data;
    try
    {
        save_data = json::parse(decrypted_data);
    } catch(...)
    {
        VORTEX_ERROR("[LOAD ERROR] Save file is corrupted or wrong encryption key!");
        return;
    }

    for (VortexModel *model : snapshot->active_models) model->should_destroy = true;
    for (ParticleSystem *ps : snapshot->active_systems) ps->should_destroy = true;

    snapshot->active_models.clear();
    snapshot->active_systems.clear();

    if (save_data.contains("empty_folders"))
    {
        snapshot->explicit_empty_folders = save_data["empty_folders"].get<std::vector<std::string>>();
    }

    for (const auto &j_model : save_data["models"])
    {
        VortexModel *new_model = new VortexModel(j_model["path"], snapshot->window, j_model["name"]);

        if (new_model->shared_data)
        {
            for (size_t i = 0; i < new_model->shared_data->objects.size(); i++)
            {
                new_model->active_parts[i] = true;
            }
        }

        if (j_model.contains("folder"))
        {
            new_model->folder = j_model["folder"];
        }
        else
        {
            new_model->folder = "Scene";
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

        if (j_model.contains("rigidbody"))
        {
            new_model->rigidbody = new VortexRigidbody();
            new_model->rigidbody->vortexGameObject = new_model;
            new_model->rigidbody->vortexTransform = &new_model->transform;

            new_model->rigidbody->deserialize(j_model["rigidbody"]);
        }

        if (j_model.contains("light"))
        {
            new_model->light = new VortexLight();
            new_model->light->vortexGameObject = new_model;
            new_model->light->vortexTransform = &new_model->transform;

            new_model->light->deserialize(j_model["light"]);
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

    VortexGUI::sync_loaded_environment();

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

    if (snapshot->settings_snapshot) load_project_settings(snapshot->settings_snapshot, snapshot->project_name);

    VORTEX_INFO("[PROJECT] Successfully loaded ", snapshot->project_name);
}

void VortexProject::load_project_settings(SettingsSnapshot *settings_snapshot, const std::string project_name)
{
    if (!settings_snapshot) return;

    memset(settings_snapshot->preferred_ide_path, 0, 256);

    if (project_name.empty()) return;

    std::string path = SAVE_DIRECTORY + "/" + project_name + "/" + project_name + "_settings.json";

    if (std::filesystem::exists(path))
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            nlohmann::json j;
            try
            {
                file >> j;
                if (j.contains("preferred_ide_path"))
                {
                    std::string ide = j["preferred_ide_path"];
                    vortex_strncpy(settings_snapshot->preferred_ide_path, 256, ide.c_str());
                }
                VORTEX_INFO("[SETTINGS] Loaded project settings: ", path);
            }
            catch (...)
            {
                VORTEX_WARN("[SETTINGS] Corrupted settings JSON: ", path);
            }
            file.close();
        }
    }
}

bool VortexProject::check_save_state(Snapshot *snapshot)
{
    std::string prefix = "temp_playmode_backup_";
    std::string folder_name = snapshot->project_name;
    if (folder_name.rfind(prefix, 0) == 0)
    {
        folder_name = folder_name.substr(prefix.length());
    }

    std::string filepath = SAVE_DIRECTORY + "/" + folder_name + "/" + snapshot->project_name + ".vtx";

    std::string decrypted_data = VortexEncrypt::read_decrypted(filepath);

    if (decrypted_data.empty())
    {
        return snapshot->active_models.empty() && snapshot->active_systems.empty();
    }

    json saved_data;
    try
    {
        saved_data = json::parse(decrypted_data);
    } catch (...)
    {
        return false;
    }

    if (saved_data["m_selected_skybox_idx"] != snapshot->m_selected_skybox_idx) return false;
    if (saved_data["m_selected_shader_idx"] != snapshot->m_selected_shader_idx) return false;

    if (saved_data["models"].size() != snapshot->active_models.size()) return false;

    std::vector<std::string> live_empty_folders = snapshot->explicit_empty_folders;
    std::vector<std::string> saved_empty_folders = {"Scene"};

    if (saved_data.contains("empty_folders"))
    {
        saved_empty_folders = saved_data["empty_folders"].get<std::vector<std::string>>();
    }

    if (live_empty_folders.size() != saved_empty_folders.size()) return false;

    for (size_t i = 0; i < live_empty_folders.size(); i++)
    {
        if (live_empty_folders[i] != saved_empty_folders[i]) return false;
    }

    if (saved_data["particle_systems"].size() != snapshot->active_systems.size()) return false;

    auto float_equals = [](float a, float b) { return std::abs(a - b) < 0.001f; };

    for (size_t i = 0; i < snapshot->active_models.size(); i++)
    {
        VortexModel* model = snapshot->active_models[i];
        json& saved_model = saved_data["models"][i];

        if (saved_model["name"] != model->model_name) return false;

        std::string live_folder = model->folder.empty() ? "Scene" : model->folder;
        std::string saved_folder = saved_model.contains("folder") ? saved_model["folder"] : "Scene";

        if (live_folder != saved_folder) return false;

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

        bool has_live_rb = (model->rigidbody != nullptr);
        bool has_saved_rb = saved_model.contains("rigidbody");

        if (has_live_rb != has_saved_rb) return false;
        if (has_live_rb && has_saved_rb)
        {
            json live_rb_data;
            model->rigidbody->serialize(live_rb_data);

            if (saved_model["rigidbody"] != live_rb_data) return false;
        }

        bool has_live_light = (model->light != nullptr);
        bool has_saved_light = saved_model.contains("light");

        if (has_live_light != has_saved_light) return false;
        if (has_live_light && has_saved_light)
        {
            json live_light_data;
            model->light->serialize(live_light_data);
            json& saved_light = saved_model["light"];

            if (!float_equals(saved_light["color"][0], model->light->color.x) ||
                !float_equals(saved_light["color"][1], model->light->color.y) ||
                !float_equals(saved_light["color"][2], model->light->color.z)) return false;

            if (!float_equals(saved_light["intensity"], model->light->intensity)) return false;
            if (!float_equals(saved_light["ambient_strength"], model->light->ambient_strength)) return false;
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
    if (project_name.empty()) project_name = std::string(VortexGUI::m_new_project_name);

    SettingsSnapshot settings_snapshot = {
        VortexGUI::preferred_ide_path
    };

    Snapshot snapshot = {
        project_name,
        VortexObjectManager::active_models,
        VortexObjectManager::active_particlesystems,
        VortexGUI::m_selected_skybox_idx,
        VortexGUI::m_selected_shader_idx,
        VortexGUI::explicit_empty_folders,
        window,
        &settings_snapshot
    };

    if (state == SnapshotState::SAVE) VortexProject::save_project(&snapshot);
    else if (state == SnapshotState::LOAD) VortexProject::load_project(&snapshot);
}

void VortexProject::clean_playmode_backups()
{
    VORTEX_INFO("[PROJECT] Purging temporary playmode cache files from backup storage directory...");

    std::string target_dir = SAVE_DIRECTORY;
    std::string prefix = "temp_playmode_backup_";

    VORTEX_INFO("[PROJECT] Scanning '" + SAVE_DIRECTORY + "/' for stale runtime playmode backups...");

    if (!std::filesystem::exists(target_dir) || !std::filesystem::is_directory(target_dir))
    {
        return;
    }

    size_t deleted_count = 0;

    try
    {
        for (const auto& project_dir : std::filesystem::directory_iterator(target_dir))
        {
            if (project_dir.is_directory())
            {
                for (const auto& file_entry : std::filesystem::directory_iterator(project_dir.path()))
                {
                    if (file_entry.is_regular_file())
                    {
                        std::string filename = file_entry.path().filename().string();

                        if (filename.rfind(prefix, 0) == 0)
                        {
                            std::filesystem::remove(file_entry.path());
                            deleted_count++;
                        }
                    }
                }
            }
        }

        if (deleted_count > 0)
        {
            VORTEX_INFO("[PROJECT] Successfully purged ", deleted_count, " temporary playmode backup files.");
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        VORTEX_ERROR("[PROJECT ERROR] Failed to clear playmode backups: ", e.what());
    }
}
