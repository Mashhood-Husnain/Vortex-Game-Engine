#include "vortex_gui.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_particlesystem.hpp"

void VortexGUI::creator_window()
{
    if (!show_creator_window) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("Creator Tools");

    if (ImGui::CollapsingHeader("File Management"))
    {
        ImGui::Spacing();
        
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

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##Select Existing", "Search existing saves..."))
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

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##ProjectName", save_project_name, sizeof(save_project_name));

        ImGui::Spacing();

        if (ImGui::Button("Save Project", ImVec2(-1, 32))) VortexProject::take_snapshot(SnapshotState::SAVE, app);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.32f, 1.0f));
        
        if (ImGui::Button("Load Project", ImVec2(-1, 32)))
        {
            VortexProject::take_snapshot(SnapshotState::LOAD, app);

            if (!m_skybox_loaded) refresh_skybox_list();
            if (!m_shaders_loaded) refresh_shader_list();

            gui_set_skybox();
            gui_set_post_processor();
        }

        if (ImGui::Button("Refresh Saves", ImVec2(-1, 24)))
        {
            all_saves = VortexProject::search_save_files();
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Spacing();
    }

    ImGui::Spacing();

    bool is_collapsed = ImGui::IsWindowCollapsed();
    if (!is_collapsed && m_creator_was_collapsed) m_force_scene_collapse = true;
    m_creator_was_collapsed = is_collapsed;

    if (!is_collapsed)
    {
        if (ImGui::CollapsingHeader("New Script"))
        {
            ImGui::Spacing();

            static char new_script_name[64] = "MyNewBehaviour";
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##ClassName", new_script_name, IM_ARRAYSIZE(new_script_name));

            static bool show_script_error = false;
            static bool show_script_success = false;

            if (ImGui::Button("Generate Script", ImVec2(-1, 32)))
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
                    script_file << "class " << class_name << " : public VortexMonoBehaviour\n{\n";
                    script_file << "private:\npublic:\n";
                    script_file << "    void on_start() override\n    {\n        // Initialization\n    }\n\n";
                    script_file << "    void on_update(float deltaTime) override\n    {\n        // Every frame\n    }\n\n";
                    script_file << "    void late_update(float deltaTime) override\n    {\n        // End of frame\n    }\n\n";
                    script_file << "    void on_message(const std::string &message, void *data) override\n    {\n        // Cross-script communication\n    }\n};\n\n";
                    script_file << "VORTEX_REGISTER_SCRIPT(" << class_name << ");\n";
                    script_file.close();
                    
                    VORTEX_INFO("[EDITOR] Auto-Generated Script: ", file_path);
                }
            }

            if (show_script_error) ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "Error: Script already exists!");
            if (show_script_success) ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.4f, 1.0f), "Generated! Hot-reload to compile.");
            
            ImGui::Spacing();
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("New Particle System"))
        {
            ImGui::Spacing();

            static char new_ps_name[64] = "Magic_Dust";
            static int new_ps_max = 100000;

            ImGui::TextDisabled("System Name");
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText("##SystemName", new_ps_name, IM_ARRAYSIZE(new_ps_name));
            
            ImGui::TextDisabled("Max Capacity");
            ImGui::SetNextItemWidth(-FLT_MIN);
            VortexGuiLambda::ClampedInputInt("##MaxCap", &new_ps_max, 10000, 50000, 100, 1000000);

            static bool show_error_message_ps = false;

            if (ImGui::Button("Create Particle System", ImVec2(-1, 32)))
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

                if (name_exists) show_error_message_ps = true;
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
                ImGui::TextColored(ImVec4(0.9f, 0.3f, 0.3f, 1.0f), "System with this name already exists!");
            }
            
            ImGui::Spacing();
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Model Browser", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.32f, 1.0f));
            
            if (ImGui::Button("Refresh Assets Folder", ImVec2(-1, 24)) || !m_models_scanned)
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
            ImGui::PopStyleColor(3);

            ImGui::Spacing();
            
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.07f, 1.0f));
            ImGui::BeginChild("ModelBrowser", ImVec2(0, -1), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

            if (m_available_model_names.empty())
            {
                ImGui::Spacing();
                ImGui::TextDisabled("  No .obj files found in:");
                ImGui::TextDisabled("  assets/models/obj");
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));

                float button_height = ImGui::GetTextLineHeightWithSpacing() * 1.8f;
                for (size_t i = 0; i < m_available_model_names.size(); i++)
                {
                    if (ImGui::Button(m_available_model_names[i].c_str(), ImVec2(-1, button_height)))
                    {
                        VortexModel *new_model = new VortexModel(m_available_model_paths[i].c_str(), app);
                        VortexObjectManager::active_models.push_back(new_model);
                    }
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }
    }

    ImGui::End();
}
