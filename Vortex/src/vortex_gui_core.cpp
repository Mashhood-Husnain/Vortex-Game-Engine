#include "vortex_gui.hpp"
#include "vortex_objectmanager.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_application.hpp"

int VortexGUI::m_selected_shader_idx = 0;
int VortexGUI::m_selected_skybox_idx = 0;
char VortexGUI::save_project_name[128] = "";

VortexGUI::VortexGUI()
{
    
}

void VortexGUI::init(VortexApplication *app, int width, int height)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;

    ImGuiStyle &style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_WindowBg]       = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_ChildBg]        = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]        = ImVec4(0.10f, 0.10f, 0.11f, 0.98f);
    colors[ImGuiCol_Border]         = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    
    colors[ImGuiCol_FrameBg]        = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_TitleBg]        = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    
    colors[ImGuiCol_Header]         = ImVec4(0.20f, 0.20f, 0.22f, 0.00f); 
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    
    colors[ImGuiCol_Button]         = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    
    colors[ImGuiCol_CheckMark]      = ImVec4(0.60f, 0.60f, 0.62f, 1.00f);
    colors[ImGuiCol_SliderGrab]     = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.60f, 0.60f, 0.62f, 1.00f);
    
    colors[ImGuiCol_Text]           = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
    colors[ImGuiCol_TextDisabled]   = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);
    
    colors[ImGuiCol_Separator]      = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]=ImVec4(0.22f, 0.22f, 0.24f, 1.00f);

    this->app = app;

    setup_scene_fbo(width, height);

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
    if (app->get_state() == EngineState::EDITOR)
    {
        ImGuiWindowFlags toolbar_flags = ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::Begin("Toolbar", nullptr, toolbar_flags);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.32f, 0.22f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.38f, 0.26f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.45f, 0.30f, 1.00f));
        
        if (ImGui::Button("RUN GAME", ImVec2(120, 32)))
        {
            VORTEX_INFO("[ENGINE] Entering Play Mode...");

            VortexProject::take_snapshot(SnapshotState::SAVE, app, "temp_playmode_backup");

            app->set_state(EngineState::PLAY);
            app->show_mouse(false);
            app->toggle_wireframe(false);
            app->toggle_world_axis(false);
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
        
        ImGui::PopStyleColor(3);
        
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void VortexGUI::draw_exit_modal()
{
    if (!app->is_exit_modal_open()) return;

    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));
    ImGui::OpenPopup("Exit Vortex Engine");
    
    app->show_mouse(true);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Exit Vortex Engine", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Spacing();

        if (app->has_unsaved())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Warning: You have unsaved changes!");
            ImGui::Text("Are you sure you want to exit? Unsaved progress will be lost.");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Project Name:");
            ImGui::SetNextItemWidth(300.0f);
            ImGui::InputText("##ProjectName", save_project_name, IM_ARRAYSIZE(save_project_name));

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("Save & Exit", ImVec2(120, 30)))
            {
                VortexProject::take_snapshot(SnapshotState::SAVE, app);

                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Exit Without Saving", ImVec2(160, 30)))
            {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(100, 30)))
            {
                ImGui::CloseCurrentPopup();
                app->toggle_exit_modal(false);
            }
        }
        else
        {
            ImGui::Text("Project is saved.");
            ImGui::Text("Are you sure you want to exit Vortex Engine?");
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Yes, Exit", ImVec2(120, 30)))
            {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 30)))
            {
                ImGui::CloseCurrentPopup();
                app->toggle_exit_modal(false);
            }
        }
        
        ImGui::Spacing();
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
}

VortexGUI::~VortexGUI()
{
    glDeleteFramebuffers(1, &scene_fbo);
    glDeleteTextures(1, &scene_texture);
    glDeleteRenderbuffers(1, &scene_rbo);

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
    m_selected_model = nullptr;    
}
