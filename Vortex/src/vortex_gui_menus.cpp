#include "vortex_gui.hpp"
#include "vortex_application.hpp"
#include "vortex_save_load.hpp"

void VortexGUI::draw_main_menu_bar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Project", "Ctrl+N"))
            {
                if (strlen(m_new_project_name) > 0)
                {
                    VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
                }
                memset(m_new_project_name, 0, sizeof(m_new_project_name));

                app->set_state(EngineState::PROJECT_HUB);
            }
            
            if (ImGui::MenuItem("Open Project", "Ctrl+O"))
            {
                if (strlen(m_new_project_name) > 0)
                {
                    VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
                }

                app->set_state(EngineState::PROJECT_HUB);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save Project", "Ctrl+S"))
            {
                VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Escape"))
            {
                if (strlen(m_new_project_name) > 0)
                {
                    VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
                }
                app->request_exit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            // TODO
            ImGui::MenuItem("Undo", "Ctrl+Z");
            ImGui::MenuItem("Redo", "Ctrl+Y");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tool"))
        {
            if (ImGui::MenuItem("Run", "Ctrl+R"))
            {
                app->enter_play_mode();
            }
            
            if (ImGui::MenuItem("Compile", "Ctrl+C"))
            {
                app->trigger_compile();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("Scene Inspector", NULL, &show_inspector);
            ImGui::MenuItem("Camera Info", NULL, &show_camera_info);
            ImGui::MenuItem("Creator Window", NULL, &show_creator_window);
            ImGui::MenuItem("Engine Stats", NULL, &show_engine_stats);
            ImGui::MenuItem("Terminal Output", NULL, &show_terminal);
            ImGui::MenuItem("Skybox/Post-Process", NULL, &show_skybox_post_process_options);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void VortexGUI::draw_project_hub()
{
    bool open_delete_modal = false;
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_Appearing);

    ImGuiWindowFlags hub_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar;

    ImGui::Begin("Vortex Project Hub", nullptr, hub_flags);

    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "VORTEX ENGINE");
    ImGui::TextDisabled("Project Launcher");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "HubColumns", false);
    ImGui::SetColumnWidth(0, 400.0f);

    ImGui::Text("Recent Projects");
    ImGui::Spacing();

    ImGui::BeginChild("ProjectList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
    
    bool found_valid_project = false;
    static bool show_overwrite_error = false;

    if (std::filesystem::exists(VortexProject::SAVE_DIRECTORY) && std::filesystem::is_directory(VortexProject::SAVE_DIRECTORY))
    {
        for (const auto& entry : std::filesystem::directory_iterator(VortexProject::SAVE_DIRECTORY))
        {
            if (entry.path().extension() == ".vtx")
            {
                std::string project_name = entry.path().stem().string();

                if (project_name.rfind("temp_playmode_backup_", 0) == 0)
                {
                    continue;
                }

                found_valid_project = true;
                
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));

                float available_width = ImGui::GetContentRegionAvail().x;
                float delete_btn_width = 30.0f;
                float selectable_width = available_width - delete_btn_width - 8.0f;

                if (ImGui::Selectable((project_name + "##" + project_name).c_str(), false, 0, ImVec2(selectable_width, 30)))
                {
                    strncpy(m_new_project_name, project_name.c_str(), sizeof(m_new_project_name) - 1);
                    m_new_project_name[sizeof(m_new_project_name) - 1] = '\0';

                    VortexObjectManager::clear_scene();
                    VortexGUI::m_selected_models.clear();
                    VortexGUI::explicit_empty_folders = {"Scene"};

                    VortexProject::take_snapshot(SnapshotState::LOAD, app, m_new_project_name);
                    app->set_state(EngineState::EDITOR);
                }

                ImGui::SameLine(selectable_width + 8.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 0.7f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));

                if (ImGui::Button(("X##del_" + project_name).c_str(), ImVec2(delete_btn_width, 30)))
                {
                    strncpy(m_project_to_delete, project_name.c_str(), sizeof(m_project_to_delete) - 1);
                    m_project_to_delete[sizeof(m_project_to_delete) - 1] = '\0';
                    
                    open_delete_modal = true;
                }

                ImGui::PopStyleColor(3);
                ImGui::PopStyleVar();
            }
        }
    }
    
    if (!found_valid_project)
    {
        ImGui::TextDisabled("No existing projects found.");
    }

    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::Text("Create New Project");
    ImGui::Spacing();

    ImGui::TextDisabled("Project Name");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##NewProjectName", m_new_project_name, IM_ARRAYSIZE(m_new_project_name));
    
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
    
    if (ImGui::Button("Create Project", ImVec2(-FLT_MIN, 40)))
    {
        if (strlen(m_new_project_name) > 0)
        {
            std::string target_path = VortexProject::SAVE_DIRECTORY + "/" + std::string(m_new_project_name) + ".vtx";

            if (std::filesystem::exists(target_path))
            {
                show_overwrite_error = true;
            }
            else
            {
                show_overwrite_error = false;
                
                VortexObjectManager::clear_scene();
                VortexGUI::m_selected_models.clear();
                VortexGUI::explicit_empty_folders = {"Scene"};

                VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
                app->set_state(EngineState::EDITOR);
            }
        }
    }
    ImGui::PopStyleColor(3);

    if (show_overwrite_error)
    {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: A project with this name already exists!");
    }

    ImVec2 modal_center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(modal_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (open_delete_modal)
    {
        ImGui::OpenPopup("Delete Confirmation");
    }

    if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Are you sure you want to delete '%s'?", m_project_to_delete);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "This action cannot be undone!");
        
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Yes, Delete", ImVec2(120, 0)))
        {
            std::string path_to_delete = VortexProject::SAVE_DIRECTORY + "/" + std::string(m_project_to_delete) + ".vtx";
            
            try
            {
                std::filesystem::remove(path_to_delete);
                VORTEX_INFO("[PROJECT] Successfully deleted: ", path_to_delete);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                VORTEX_ERROR("[PROJECT ERROR] Failed to delete project: ", e.what());
            }

            memset(m_project_to_delete, 0, sizeof(m_project_to_delete));
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            memset(m_project_to_delete, 0, sizeof(m_project_to_delete));
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}
