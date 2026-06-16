#include "vortex_gui.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_particlesystem.hpp"

void create_script(std::string file_path, std::string class_name)
{
    std::ofstream script_file(file_path);
    script_file << "#include \"VortexEngine.hpp\"\n\n";
    script_file << "class " << class_name << " : public VortexMonoBehaviour\n{\n";
    script_file << "private:\npublic:\n";
    script_file << "    void on_start() override\n    {\n        // Initialization\n    }\n\n";
    script_file << "    void on_update() override\n    {\n        // Every frame\n    }\n\n";
    script_file << "    void late_update() override\n    {\n        // End of frame\n    }\n\n";
    script_file << "    void on_message(const std::string &message, void *data) override\n    {\n        // Cross-script communication\n    }\n};\n\n";
    script_file << "VORTEX_REGISTER_SCRIPT(" << class_name << ");\n";
    script_file.close();

    VORTEX_INFO("[EDITOR] Auto-Generated Script: ", file_path);
}

void VortexGUI::creator_window()
{
    if (!show_creator_window) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Creator Tools");

    ImGui::Spacing();

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

            std::string file_path = vortex_generatepath(
                VortexProject::SAVE_DIRECTORY,
                m_new_project_name,
                VortexProject::ASSET_DIR_SCRIPTS,
                class_name + ".cpp"
            );

            if (std::filesystem::exists(file_path))
            {
                show_script_error = true;
                show_script_success = false;
            }
            else if (!class_name.empty())
            {
                show_script_error = false;
                show_script_success = true;

                create_script(file_path, class_name);
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

        std::string user_models_path = vortex_generatepath(
            VortexProject::SAVE_DIRECTORY,
            m_new_project_name,
            VortexProject::ASSET_DIR_MODELS,
            VortexProject::ASSET_DIR_MODELS_OBJ
        );
        std::string engine_models_path = "assets/models/obj";

        static std::vector<std::pair<std::string, std::string>> cached_user_models;
        static std::vector<std::pair<std::string, std::string>> cached_engine_models;

        if (ImGui::Button("Refresh Assets Folder", ImVec2(-1, 24)) || !m_models_scanned)
        {
            cached_user_models.clear();
            cached_engine_models.clear();

            if (std::filesystem::exists(user_models_path) && std::filesystem::is_directory(user_models_path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(user_models_path))
                {
                    if (entry.path().extension() == ".obj")
                    {
                        cached_user_models.push_back({entry.path().stem().string(), entry.path().string()});
                    }
                }
            }

            if (std::filesystem::exists(engine_models_path) && std::filesystem::is_directory(engine_models_path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(engine_models_path))
                {
                    if (entry.path().extension() == ".obj")
                    {
                        cached_engine_models.push_back({entry.path().stem().string(), entry.path().string()});
                    }
                }
            }

            m_models_scanned = true;
        }
        ImGui::PopStyleColor(3);

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.07f, 1.0f));

        ImVec2 child_size = ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 5.0f);
        ImGui::BeginChild("ModelBrowser", child_size, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        auto draw_model_category = [&](const char* title, const std::vector<std::pair<std::string, std::string>>& models)
        {
            if (ImGui::TreeNodeEx(title, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                if (models.empty())
                {
                    ImGui::Spacing();
                    ImGui::TextDisabled("   No models found.");
                    ImGui::Spacing();
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.20f, 0.22f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));

                    float button_height = ImGui::GetTextLineHeightWithSpacing() * 1.5f;
                    for (const auto& model_pair : models)
                    {
                        std::string button_label = "  " + model_pair.first;

                        if (ImGui::Button(button_label.c_str(), ImVec2(-1, button_height)))
                        {
                            VortexModel *new_model = new VortexModel(model_pair.second.c_str(), app);
                            new_model->folder = "Scene";
                            VortexObjectManager::active_models.push_back(new_model);
                            ActionManager::push_action(new ActionCreate(new_model));
                        }
                    }
                    ImGui::PopStyleColor(3);
                }
                ImGui::TreePop();
            }
        };

        draw_model_category("User Models", cached_user_models);
        ImGui::Spacing();
        draw_model_category("Engine Models", cached_engine_models);

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    ImGui::End();
}
