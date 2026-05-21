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
                app->set_state(EngineState::PROJECT_HUB);
            }
            
            if (ImGui::MenuItem("Open Project", "Ctrl+O"))
            {
                app->set_state(EngineState::PROJECT_HUB);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save Project", "Ctrl+S"))
            {
                VortexProject::take_snapshot(SnapshotState::SAVE, app, save_project_name);
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit", "Escape"))
            {
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
    
    if (std::filesystem::exists("saves") && std::filesystem::is_directory("saves"))
    {
        for (const auto& entry : std::filesystem::directory_iterator("saves"))
        {
            if (entry.path().extension() == ".vtx")
            {
                std::string project_name = entry.path().stem().string();
                
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
                if (ImGui::Selectable((project_name + "##" + project_name).c_str(), false, 0, ImVec2(0, 30)))
                {
                    strncpy(save_project_name, project_name.c_str(), sizeof(save_project_name) - 1);
                    save_project_name[sizeof(save_project_name) - 1] = '\0';

                    VortexProject::take_snapshot(SnapshotState::LOAD, app, project_name);
                    app->set_state(EngineState::EDITOR);
                }
                ImGui::PopStyleVar();
            }
        }
    }
    else
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
            VortexProject::take_snapshot(SnapshotState::SAVE, app, m_new_project_name);
            
            memset(m_new_project_name, 0, sizeof(m_new_project_name));
            app->set_state(EngineState::EDITOR);
        }
    }
    ImGui::PopStyleColor(3);

    ImGui::End();
}
