#include "vortex_gui.hpp"
#include "vortex_objectmanager.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_application.hpp"

VortexGUI::VortexGUI()
{

}

void VortexGUI::init(VortexApplication *app)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.IniFilename = nullptr;

    ImGuiStyle &style = ImGui::GetStyle();

    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;

    ImVec4 *colors = style.Colors;

    this->app = app;

    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(app->get_window_ptr(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void VortexGUI::update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_processed_models.clear();
    m_processed_ps.clear();
}

void VortexGUI::render()
{
    if (app->current_state == EngineState::EDITOR)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 10.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        
        ImGui::SetNextWindowBgAlpha(0.8f); 

        ImGuiWindowFlags toolbar_flags = 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoNav | 
            ImGuiWindowFlags_NoMove;

        ImGui::Begin("Toolbar", nullptr, toolbar_flags);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        
        if (ImGui::Button("RUN GAME", ImVec2(100, 30)))
        {
            VORTEX_INFO("[ENGINE] Entering Play Mode...");

            std::string project_name = "temp_playmode_backup";
            SaveScene_snapshot snapshot = {
                project_name,
                VortexObjectManager::active_models,
                VortexObjectManager::active_particlesystems,
                m_selected_skybox_idx,
                m_selected_shader_idx,
                app
            };

            VortexProject::save_project(&snapshot);

            app->current_state = EngineState::PLAY;
            app->show_mouse(false);
            app->show_wireframe = false;
            app->view_world_axis = false;
            show_gui = false;
            show_debug_gui = false;

            for (VortexModel *model : VortexObjectManager::active_models)
            {
                for (VortexMonoBehaviour *script : model->behaviours)
                {
                    script->on_start();
                }
            }
        }
        
        ImGui::PopStyleColor(2);
        
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void VortexGUI::draw_exit_modal()
{
    if (!app->show_exit_modal) return;

    ImGui::OpenPopup("Exit Vortex Engine");
    app->show_mouse(true);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Exit Vortex Engine", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        if (app->has_unsaved_changes)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Warning: You have unsaved changes!");
            ImGui::Text("Are you sure you want to exit? Unsaved progress will be lost.");
            ImGui::Separator();

            ImGui::Text("Project Name:");
            ImGui::InputText("##ProjectName", save_project_name, IM_ARRAYSIZE(save_project_name));

            ImGui::Spacing();

            if (ImGui::Button("Save & Exit", ImVec2(120, 0)))
            {
                std::string project_name(save_project_name);

                SaveScene_snapshot snapshot = {
                    project_name,
                    VortexObjectManager::active_models,
                    VortexObjectManager::active_particlesystems,
                    m_selected_skybox_idx,
                    m_selected_shader_idx,
                    app
                };

                VortexProject::save_project(&snapshot);

                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Exit Without Saving", ImVec2(150, 0)))
            {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                app->show_exit_modal = false;
            }
        }
        else
        {
            ImGui::Text("Project is saved.");
            ImGui::Text("Are you sure you want to exit Vortex Engine?");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Yes, Exit", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                app->show_exit_modal = false;
            }
        }
        
        ImGui::EndPopup();
    }
}

VortexGUI::~VortexGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_processed_models.clear();
    m_processed_ps.clear();

    m_shader_files.clear();
    m_display_names.clear();

    for (std::vector<std::string> file : m_skybox_files)
    {
        file.clear();
    }
    m_skybox_files.clear();
    m_skybox_display_names.clear();

    m_available_model_names.clear();
    m_available_model_paths.clear();

    app = nullptr;
}
