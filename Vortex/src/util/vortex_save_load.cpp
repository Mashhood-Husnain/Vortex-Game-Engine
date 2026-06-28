#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

std::string get_global_backup_directory()
{
    std::string path;

    #if defined(_WIN32) || defined(_WIN64)
        char path_buffer[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path_buffer)))
        {
            path = std::string(path_buffer) + "\\VortexEngine\\backups";
            std::replace(path.begin(), path.end(), '\\', '/');
        }
        else
        {
            path = "C:/VortexEngine/backups"; 
        }
    #else
        const char *homedir;

        if ((homedir = getenv("HOME")) == NULL) {
            homedir = getpwuid(getuid())->pw_dir;
        }

        path = std::string(homedir) + "/.local/share/VortexEngine/backups";
    #endif

    return path;
}

void VortexProject::create_backup(const std::string& project_name)
{
    if (project_name.empty() || project_name.find("temp_playmode_backup_") == 0) return;

    std::string src_dir = vortex_generatepath(SAVE_DIRECTORY, project_name);
    if (!std::filesystem::exists(src_dir)) return;

    std::string backup_base_dir = get_global_backup_directory();
    if (!std::filesystem::exists(backup_base_dir))
    {
        std::filesystem::create_directories(backup_base_dir);
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), "%Y%m%d_%H%M%S", &local_tm);

    std::string backup_folder_name = project_name + "_backup_" + std::string(time_buf);
    std::string dest_dir = vortex_generatepath(backup_base_dir, backup_folder_name);

    try
    {
        std::filesystem::copy(src_dir, dest_dir, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
        VORTEX_INFO("[PROJECT] Backup securely archived at: ", dest_dir);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        VORTEX_ERROR("[PROJECT ERROR] Failed to create backup: ", e.what());
    }
}

void generate_ide_context(const std::string& project_name)
{
    std::string scripts_dir = vortex_generatepath(
        VortexProject::SAVE_DIRECTORY,
        project_name,
        VortexProject::ASSET_DIR_SCRIPTS
    );

    std::string cmake_path = vortex_generatepath(scripts_dir, "CMakeLists.txt");
    std::string vscode_dir = vortex_generatepath(scripts_dir, ".vscode");
    std::string cpp_props_path = vortex_generatepath(vscode_dir, "c_cpp_properties.json");

    if (std::filesystem::exists(cmake_path) && std::filesystem::exists(cpp_props_path)) return;

    std::string engine_root = std::filesystem::absolute("..").string();
    std::replace(engine_root.begin(), engine_root.end(), '\\', '/');

    std::vector<std::string> include_paths;
    include_paths.push_back(engine_root + "/Vortex/include");
    include_paths.push_back(engine_root + "/Vortex/include/util");

    std::string vendor_path = engine_root + "/vendor";
    if (std::filesystem::exists(vendor_path) && std::filesystem::is_directory(vendor_path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(vendor_path))
        {
            if (entry.is_directory())
            {
                std::string path_to_add = entry.path().string();
                std::string inc_dir = path_to_add + "/include";

                if (std::filesystem::exists(inc_dir)) path_to_add = inc_dir;

                std::replace(path_to_add.begin(), path_to_add.end(), '\\', '/');
                include_paths.push_back(path_to_add);
            }
        }
    }

    if (!std::filesystem::exists(cmake_path))
    {
        std::ofstream cmake_file(cmake_path);
        if (cmake_file.is_open())
        {
            cmake_file << "cmake_minimum_required(VERSION 3.10)\n";
            cmake_file << "project(" << project_name << "_Scripts)\n\n";
            cmake_file << "set(CMAKE_CXX_STANDARD 20)\n\n";
            cmake_file << "file(GLOB_RECURSE USER_SCRIPTS \"*.cpp\")\n";
            cmake_file << "add_library(DummyContext SHARED ${USER_SCRIPTS})\n\n";
            cmake_file << "target_include_directories(DummyContext PUBLIC\n";

            for (const std::string& path : include_paths)
            {
                cmake_file << "    \"" << path << "\"\n";
            }

            cmake_file << ")\n";
            cmake_file.close();
        }
    }

    if (!std::filesystem::exists(vscode_dir))
    {
        std::filesystem::create_directory(vscode_dir);
    }

    if (!std::filesystem::exists(cpp_props_path))
    {
        std::ofstream props_file(cpp_props_path);
        if (props_file.is_open())
        {
            props_file << "{\n";
            props_file << "    \"configurations\": [\n";
            props_file << "        {\n";
            props_file << "            \"name\": \"VortexEngine\",\n";
            props_file << "            \"includePath\": [\n";
            props_file << "                \"${workspaceFolder}/**\",\n";

            for (size_t i = 0; i < include_paths.size(); i++)
            {
                props_file << "                \"" << include_paths[i] << "\"";
                if (i < include_paths.size() - 1) props_file << ",";
                props_file << "\n";
            }

            props_file << "            ],\n";
            props_file << "            \"defines\": [],\n";

            #if defined(_WIN32) || defined(_WIN64)
                props_file << "            \"intelliSenseMode\": \"windows-gcc-x64\",\n";
            #else
                props_file << "            \"intelliSenseMode\": \"linux-gcc-x64\",\n";
            #endif

            props_file << "            \"cStandard\": \"c17\",\n";
            props_file << "            \"cppStandard\": \"c++20\"\n";
            props_file << "        }\n";
            props_file << "    ],\n";
            props_file << "    \"version\": 4\n";
            props_file << "}\n";
            props_file.close();
        }
    }

    VORTEX_INFO("[PROJECT] Generated IDE context for project: ", project_name);
}

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

    std::string project_save_dir = vortex_generatepath(SAVE_DIRECTORY, folder_name);
    if (!std::filesystem::exists(project_save_dir))
    {
        std::filesystem::create_directory(project_save_dir);
    }

    std::string project_asset_dir = vortex_generatepath(project_save_dir, ASSET_DIR_SCRIPTS);
    if (!std::filesystem::exists(project_asset_dir))
    {
        std::filesystem::create_directory(project_asset_dir);
    }

    std::string project_audio_dir = vortex_generatepath(project_save_dir, ASSET_DIR_AUDIO);
    if (!std::filesystem::exists(project_audio_dir))
    {
        std::filesystem::create_directory(project_audio_dir);
    }

    std::string project_models_dir = vortex_generatepath(project_save_dir, ASSET_DIR_MODELS);
    if (!std::filesystem::exists(project_models_dir))
    {
        std::filesystem::create_directory(project_models_dir);
    }

    std::string project_models_obj_dir = vortex_generatepath(project_models_dir, ASSET_DIR_MODELS_OBJ);
    if (!std::filesystem::exists(project_models_obj_dir))
    {
        std::filesystem::create_directory(project_models_obj_dir);
    }

    std::string project_models_mtl_dir = vortex_generatepath(project_models_dir, ASSET_DIR_MODELS_MTL);
    if (!std::filesystem::exists(project_models_mtl_dir))
    {
        std::filesystem::create_directory(project_models_mtl_dir);
    }

    std::string project_models_mtl_textures_dir = vortex_generatepath(project_models_mtl_dir, ASSET_DIR_MODELS_MTL_TEXTURES);
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

        j_model["texture_path"] = model->texture_path;
        j_model["texture_scale"] = {model->texture_scale.x, model->texture_scale.y};

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

        if (!model->decals.empty())
        {
            json d_data = json::array();
            for (VortexDecal* decal : model->decals)
            {
                json d_json;
                decal->serialize(d_json);
                d_data.push_back(d_json);
            }
            j_model["decals"] = d_data;
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

    std::string file_path = vortex_generatepath(project_save_dir, snapshot->project_name + ".vtx");

    std::string json_string = save_data.dump(4);
    VortexEncrypt::write_encrypted(file_path, json_string);

    if (snapshot->settings_snapshot) save_project_settings(snapshot->settings_snapshot, snapshot->project_name);

    VORTEX_INFO("[PROJECT] Successfully saved to: ", file_path);
}

void VortexProject::save_project_settings(SettingsSnapshot *settings_snapshot, const std::string project_name)
{
    if (!settings_snapshot) return;
    if (project_name.empty() || project_name.find("temp_playmode_backup_") == 0) return;

    std::string path = vortex_generatepath(SAVE_DIRECTORY, project_name, project_name + "_settings.json");

    nlohmann::json j;
    j["preferred_ide_path"] = std::string(settings_snapshot->preferred_ide_path);

    nlohmann::json layout_j;
    layout_j["show_inspector"] = settings_snapshot->engine_layout.show_inspector;
    layout_j["show_creator_window"] = settings_snapshot->engine_layout.show_creator_window;
    layout_j["show_terminal"] = settings_snapshot->engine_layout.show_terminal;
    layout_j["show_skybox_post_process_options"] = settings_snapshot->engine_layout.show_skybox_post_process_options;
    layout_j["show_scene_viewport"] = settings_snapshot->engine_layout.show_scene_viewport;
    layout_j["show_render_scene_viewport"] = settings_snapshot->engine_layout.show_render_scene_viewport;
    layout_j["show_asset_browser"] = settings_snapshot->engine_layout.show_asset_browser;
    layout_j["show_camera_info"] = settings_snapshot->engine_layout.show_camera_info;
    layout_j["show_engine_stats"] = settings_snapshot->engine_layout.show_engine_stats;
    layout_j["show_stack_history_window"] = settings_snapshot->engine_layout.show_stack_history_window;
    layout_j["show_settings_window"] = settings_snapshot->engine_layout.show_settings_window;
    layout_j["show_file_viewer"] = settings_snapshot->engine_layout.show_file_viewer;
    layout_j["show_image_viewer"] = settings_snapshot->engine_layout.show_image_viewer;

    layout_j["imgui_ini_data"] = settings_snapshot->engine_layout.imgui_ini_data;

    j["window_layout"] = layout_j;

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

    std::string file_path = vortex_generatepath(SAVE_DIRECTORY, folder_name, snapshot->project_name  + ".vtx");
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

        if (j_model.contains("texture_path"))
        {
            std::string saved_path = j_model["texture_path"];
            if (!saved_path.empty())
            {
                new_model->texture_path = saved_path;
                new_model->texture_id = VortexAssetManager::load_texture_raw(saved_path);
            }
        }

        if (j_model.contains("texture_scale"))
        {
            new_model->texture_scale = glm::vec2(j_model["texture_scale"][0], j_model["texture_scale"][1]);
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

        if (j_model.contains("decals"))
        {
            for (const json& d_json : j_model["decals"])
            {
                VortexDecal* new_decal = new VortexDecal();
                new_decal->vortexGameObject = new_model;
                new_decal->vortexEngine = snapshot->window;
                new_decal->vortexTransform = &new_model->transform;

                new_decal->on_start();
                new_decal->deserialize(d_json);

                new_model->decals.push_back(new_decal);
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
    if (!settings_snapshot || project_name.empty()) return;

    std::string prefix = "temp_playmode_backup_";
    std::string actual_project_name = project_name;
    if (actual_project_name.rfind(prefix, 0) == 0)
    {
        actual_project_name = actual_project_name.substr(prefix.length());
    }

    memset(settings_snapshot->preferred_ide_path, 0, 256);

    std::string path = vortex_generatepath(SAVE_DIRECTORY, actual_project_name, actual_project_name + "_settings.json");

    if (std::filesystem::exists(path))
    {
        std::ifstream file(path);
        if (file.is_open())
        {
            nlohmann::json j;
            try
            {
                file >> j;

                if (j.contains("preferred_ide_path") && j["preferred_ide_path"].is_string())
                {
                    std::string ide = j["preferred_ide_path"];
                    vortex_strncpy(settings_snapshot->preferred_ide_path, 256, ide.c_str());
                }

                if (j.contains("window_layout"))
                {
                    auto& layout_j = j["window_layout"];
                    settings_snapshot->engine_layout.show_inspector = layout_j.value("show_inspector", true);
                    settings_snapshot->engine_layout.show_creator_window = layout_j.value("show_creator_window", true);
                    settings_snapshot->engine_layout.show_terminal = layout_j.value("show_terminal", true);
                    settings_snapshot->engine_layout.show_skybox_post_process_options = layout_j.value("show_skybox_post_process_options", true);
                    settings_snapshot->engine_layout.show_scene_viewport = layout_j.value("show_scene_viewport", true);
                    settings_snapshot->engine_layout.show_render_scene_viewport = layout_j.value("show_render_scene_viewport", false);
                    settings_snapshot->engine_layout.show_asset_browser = layout_j.value("show_asset_browser", true);
                    settings_snapshot->engine_layout.show_camera_info = layout_j.value("show_camera_info", false);
                    settings_snapshot->engine_layout.show_engine_stats = layout_j.value("show_engine_stats", false);
                    settings_snapshot->engine_layout.show_stack_history_window = layout_j.value("show_stack_history_window", false);
                    settings_snapshot->engine_layout.show_settings_window = layout_j.value("show_settings_window", false);
                    settings_snapshot->engine_layout.show_file_viewer = layout_j.value("show_file_viewer", false);
                    settings_snapshot->engine_layout.show_image_viewer = layout_j.value("show_image_viewer", false);

                    settings_snapshot->engine_layout.imgui_ini_data = layout_j.value("imgui_ini_data", "");
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

    std::string filepath = vortex_generatepath(SAVE_DIRECTORY, folder_name, snapshot->project_name + ".vtx");

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

    size_t ini_size = 0;
    std::string imgui_ini_data = "";
    const char *ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
    if (ini_data)
    {
        imgui_ini_data = std::string(ini_data, ini_size);
    }

    SettingsSnapshot settings_snapshot = {
        VortexGUI::preferred_ide_path,
        {
            VortexGUI::show_inspector,
            VortexGUI::show_camera_info,
            VortexGUI::show_creator_window,
            VortexGUI::show_engine_stats,
            VortexGUI::show_terminal,
            VortexGUI::show_skybox_post_process_options,
            VortexGUI::show_scene_viewport,
            VortexGUI::show_render_scene_viewport,
            VortexGUI::show_stack_history_window,
            VortexGUI::show_asset_browser,
            VortexGUI::show_settings_window,
            VortexGUI::show_file_viewer,
            VortexGUI::show_image_viewer,
            imgui_ini_data
        }
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

    if (state == SnapshotState::SAVE)
    {
        if (project_name.find("temp_playmode_backup_") == std::string::npos)
        {
            std::string base_path = vortex_generatepath(SAVE_DIRECTORY, project_name);
            if (!std::filesystem::exists(base_path)) std::filesystem::create_directory(base_path);

            std::string scripts_path = vortex_generatepath(base_path, ASSET_DIR_SCRIPTS);
            if (!std::filesystem::exists(scripts_path)) std::filesystem::create_directory(scripts_path);

            generate_ide_context(project_name);
        }

        VortexProject::save_project(&snapshot);
    }
    else if (state == SnapshotState::LOAD)
    {
        VortexProject::load_project(&snapshot);

        VortexGUI::show_inspector = settings_snapshot.engine_layout.show_inspector;
        VortexGUI::show_creator_window = settings_snapshot.engine_layout.show_creator_window;
        VortexGUI::show_terminal = settings_snapshot.engine_layout.show_terminal;
        VortexGUI::show_skybox_post_process_options = settings_snapshot.engine_layout.show_skybox_post_process_options;
        VortexGUI::show_scene_viewport = settings_snapshot.engine_layout.show_scene_viewport;
        VortexGUI::show_render_scene_viewport = settings_snapshot.engine_layout.show_render_scene_viewport;
        VortexGUI::show_asset_browser = settings_snapshot.engine_layout.show_asset_browser;
        VortexGUI::show_camera_info = settings_snapshot.engine_layout.show_camera_info;
        VortexGUI::show_engine_stats = settings_snapshot.engine_layout.show_engine_stats;
        VortexGUI::show_stack_history_window = settings_snapshot.engine_layout.show_stack_history_window;
        VortexGUI::show_settings_window = settings_snapshot.engine_layout.show_settings_window;
        VortexGUI::show_file_viewer = settings_snapshot.engine_layout.show_file_viewer;
        VortexGUI::show_image_viewer = settings_snapshot.engine_layout.show_image_viewer;

        if (!settings_snapshot.engine_layout.imgui_ini_data.empty())
        {
            VORTEX_INFO("[GUI] Queuing layout restoration (", settings_snapshot.engine_layout.imgui_ini_data.size(), " bytes)");
            VortexGUI::pending_ini_data = settings_snapshot.engine_layout.imgui_ini_data;
            VortexGUI::pending_layout_load = true;
        }
    }
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
