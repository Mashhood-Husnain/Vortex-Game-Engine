#include "vortex_gui.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_particlesystem.hpp"

void VortexGUI::creator_window()
{
    if (!show_gui) return;

    float padding = 10.0f;
    ImVec2 next_pos = ImVec2(m_scene_pos.x, m_scene_pos.y + m_scene_size.y + padding);
    ImGui::SetNextWindowPos(next_pos, ImGuiCond_Always);

    float app_window_height = ImGui::GetIO().DisplaySize.y;
    float max_height = app_window_height - next_pos.y - 10.0f;
    if (max_height < 50.0f) max_height = 50.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 50), ImVec2(FLT_MAX, max_height));

    if (m_force_creator_collapse)
    {
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
        m_force_creator_collapse = false;
    }

    ImGui::Begin("Creator Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::SeparatorText("File Management");
    
    static std::vector<std::string> all_saves;
    static bool loaded_once = false;

    if (!loaded_once)
    {
        all_saves = VortexProject::search_save_files();
        loaded_once = true;
    }

    std::vector<std::string> filtered_saves;
    std::string current_input = std::string(save_project_name);

    for (const auto &file : all_saves)
    {
        std::string name = std::filesystem::path(file).stem().string();

        if (current_input.empty() || name.find(current_input) != std::string::npos)
        {
            filtered_saves.push_back(name);
        }
    }

    if (ImGui::BeginCombo("Select Existing", "Choose save file"))
    {
        for (const auto &name : filtered_saves)
        {
            bool is_selected = (current_input == name);

            if (ImGui::Selectable(name.c_str(), is_selected))
            {
                snprintf(save_project_name, sizeof(save_project_name), "%s", name.c_str());
            }

            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::Button("Save Project", ImVec2(160, 30)))
    {
        std::string project_name = std::string(save_project_name);
        SaveScene_snapshot snapshot = {
            project_name,
            VortexObjectManager::active_models,
            VortexObjectManager::active_particlesystems,
            m_selected_skybox_idx,
            m_selected_shader_idx,
            app
        };
        
        VortexProject::save_project(&snapshot);
    }
    if (ImGui::Button("Load Project", ImVec2(160, 30)))
    {
        std::string project_name = std::string(save_project_name);
        SaveScene_snapshot snapshot = {
            project_name,
            VortexObjectManager::active_models,
            VortexObjectManager::active_particlesystems,
            m_selected_skybox_idx,
            m_selected_shader_idx,
            app
        };

        VortexProject::load_project(&snapshot);

        if (!m_skybox_loaded) refresh_skybox_list();
        if (!m_shaders_loaded) refresh_shader_list();

        gui_set_skybox();
        gui_set_post_processor();
    }

    if (ImGui::Button("Refresh Saves"))
    {
        all_saves = VortexProject::search_save_files();
    }

    ImGui::Spacing();

    bool is_collapsed = ImGui::IsWindowCollapsed();
    if (!is_collapsed && m_creator_was_collapsed)
    {
        m_force_scene_collapse = true;
    }
    m_creator_was_collapsed = is_collapsed;

    if (!is_collapsed)
    {
        ImGui::SeparatorText("New Script");

        static char new_script_name[64] = "MyNewBehaviour";
        ImGui::InputText("Class Name", new_script_name, IM_ARRAYSIZE(new_script_name));

        static bool show_script_error = false;
        static bool show_script_success = false;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.3f, 0.8f, 1.0f));

        if (ImGui::Button("Generate Script", ImVec2(-1, 0)))
        {
            std::string class_name = std::string(new_script_name);
            std::string file_path = "../Vortex/Game/Scripts/" + class_name + ".cpp";

            if (std::filesystem::exists(file_path))
            {
                show_script_error = true;
                show_script_success = false;
            }
            else if (!class_name.empty())
            {
                show_script_error = false;
                show_script_success = true;

                std::ofstream script_file(file_path);

                script_file << "#include \"VortexEngine.hpp\"\n\n";
                
                script_file << "class " << class_name << " : public VortexMonoBehaviour\n";
                script_file << "{\n";
                script_file << "private:\n";
                script_file << "public:\n";
                script_file << "    void on_start() override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run once as initialization\n";
                script_file << "    }\n\n";
                script_file << "    void on_update(float deltaTime) override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run every frame\n";
                script_file << "    }\n\n";
                script_file << "    void late_update(float deltaTime) override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run at the end of every frame\n";
                script_file << "    }\n\n";
                script_file << "    void on_message(const std::string &message, void *data) override\n";
                script_file << "    {\n";
                script_file << "        // write code here to receive communicate with scripts\n";
                script_file << "        // to send message to other object us the model->send_message(message, &data) function\n";
                script_file << "    }\n";
                script_file << "};\n\n";
                
                script_file << "VORTEX_REGISTER_SCRIPT(" << class_name << ");\n";
                
                script_file.close();
                
                VORTEX_INFO("[EDITOR] Auto-Generated Script: ", file_path);
            }
        }
        ImGui::PopStyleColor(2);

        if (show_script_error)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Error: Script already exists!");
        }
        if (show_script_success)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Generated! Restart engine to compile.");
        }
        ImGui::Spacing();

        ImGui::SeparatorText("New Particle System");

        static char new_ps_name[64] = "Magic_Dust";
        static int new_ps_max = 100000;

        ImGui::InputText("System Name", new_ps_name, IM_ARRAYSIZE(new_ps_name));
        VortexGuiLambda::ClampedInputInt("Max Capacity", &new_ps_max, 10000, 50000, 100, 1000000);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));

        static bool show_error_message_ps = false;

        if (ImGui::Button("Create Particle System", ImVec2(-1, 0)))
        {
            std::string target_name = std::string(new_ps_name);
            bool name_exists = false;

            for (ParticleSystem *ps : VortexObjectManager::active_particlesystems)
            {
                if (ps->name == target_name)
                {
                    name_exists = true;
                    break;
                }
            }

            if (name_exists)
            {
                show_error_message_ps = true;
            }
            else
            {
                show_error_message_ps = false;

                ParticleSystem *new_ps = new ParticleSystem(new_ps_max, app, std::string(new_ps_name));
                new_ps->get_emitter("Default Emitter");
                VortexObjectManager::active_particlesystems.push_back(new_ps);
            }
        }

        if (show_error_message_ps)
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Particles System with name(id) already exists!");
        }

        ImGui::PopStyleColor(2);

        ImGui::SeparatorText("New Model");

        if (ImGui::Button("Refresh Folder") || !m_models_scanned)
        {
            m_available_model_names.clear();
            m_available_model_paths.clear();

            std::string path = "assets/models/obj";

            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(path))
                {
                    if (entry.path().extension() == ".obj")
                    {
                        m_available_model_names.push_back(entry.path().stem().string());
                        m_available_model_paths.push_back(entry.path().string());
                    }
                }
            }
            m_models_scanned = true;
        }

        ImGui::Spacing();
        ImGui::BeginChild("ModelBrowser", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        if (m_available_model_names.empty())
        {
            ImGui::TextDisabled("No .obj files found in:");
            ImGui::TextDisabled("assets/models/obj");
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));

            float button_double_height = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
            for (size_t i = 0; i < m_available_model_names.size(); i++)
            {
                if (ImGui::Button(m_available_model_names[i].c_str(), ImVec2(-1, button_double_height)))
                {
                    VortexModel *new_model = new VortexModel(m_available_model_paths[i].c_str(), app);
                    VortexObjectManager::active_models.push_back(new_model);
                }
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::EndChild();
    }

    ImGui::End();
}
