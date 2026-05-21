#include "vortex_gui.hpp"
#include "vortex_camera.hpp"

EngineStatsThreaded g_engine_stats;

void VortexGUI::engine_stats()
{
    if (!show_engine_stats) return;

    ImGui::Begin("Performance");

    if (ImGui::CollapsingHeader("Framerate", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();
        
        float fps = ImGui::GetIO().Framerate;
        
        ImVec4 fps_color = ImVec4(0.3f, 0.8f, 0.4f, 1.0f);
        if (fps < 55.0f) fps_color = ImVec4(0.8f, 0.7f, 0.2f, 1.0f);
        if (fps < 30.0f) fps_color = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);

        ImGui::TextDisabled("FPS:"); 
        ImGui::SameLine(60.0f); 
        ImGui::TextColored(fps_color, "%.1f", fps);
        
        ImGui::TextDisabled("Frame:"); 
        ImGui::SameLine(60.0f); 
        ImGui::Text("%.2f ms", 1000.0f / fps);

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::Spacing();

    EngineStats stats = g_engine_stats.get_stats();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.30f, 0.35f, 0.40f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));

    if (ImGui::CollapsingHeader("System Resources", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        ImGui::TextDisabled("CPU Usage");
        ImGui::ProgressBar(
            stats.cpu_usage / 100.0f, ImVec2(-1, 14),
            (std::to_string((int)stats.cpu_usage) + "%").c_str()
        );

        ImGui::Spacing();
        ImGui::TextDisabled("RAM Usage:"); 
        ImGui::SameLine(85.0f); 
        ImGui::Text("%.1f MB", stats.ram_usage);

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Graphics Resources", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        if (stats.gpu_usage >= 0.0f)
        {
            ImGui::TextDisabled("GPU Usage");
            ImGui::ProgressBar(
                stats.gpu_usage / 100.0f, ImVec2(-1, 14),
                (std::to_string((int)stats.gpu_usage) + "%").c_str()
            );
        }
        else 
        {
            ImGui::TextDisabled("GPU Usage:"); 
            ImGui::SameLine(85.0f); 
            ImGui::Text("N/A");
        }

        if (stats.gpu_mem_total > 0)
        {
            ImGui::Spacing();
            ImGui::TextDisabled("VRAM Usage");
            ImGui::ProgressBar(
                stats.gpu_mem_usage / stats.gpu_mem_total, ImVec2(-1, 14),
                (std::to_string((int)stats.gpu_mem_usage) + " MB").c_str()
            );
        }

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::PopStyleColor(2);
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Hardware Info"))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();
        
        std::string vendor_text = "VENDOR: " + _vendor_;
        std::string renderer_text = "RENDERER: " + _renderer_;
        
        ImVec4 muted_green = ImVec4(0.4f, 0.7f, 0.5f, 1.0f);
        ImGui::TextColored(muted_green, "%s", vendor_text.c_str());
        ImGui::TextColored(muted_green, "%s", renderer_text.c_str());

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::End();
}

void VortexGUI::camera_info(VortexCamera *camera)
{
    if (!camera || !show_camera_info) return;

    ImGui::Begin("Camera");

    if (ImGui::CollapsingHeader("Transform & Status", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        ImGui::TextDisabled("Position");
        ImGui::Text("X: %.1f  Y: %.1f  Z: %.1f", camera->get_position().x, camera->get_position().y, camera->get_position().z);

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::Spacing();
    
    if (ImGui::CollapsingHeader("Movement Settings", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();
        
        ImGui::TextDisabled("Fly Speed");
        ImGui::SetNextItemWidth(-FLT_MIN);
        float camera_movement_speed = camera->get_movement_speed();
        VortexGuiLambda::ClampedInputFloat("##Speed", &camera_movement_speed, 0.5f, 1.0f, 0.0f, 100.0f);
        camera->set_movement_speed(camera_movement_speed);

        ImGui::Spacing();
        ImGui::Unindent(10.0f);
    }

    ImGui::End();
}

void VortexGUI::draw_compiler_modal()
{
    if (CompilerState::is_compiling.load())
    {
        if (!ImGui::IsPopupOpen("Compiling Scripts"))
        {
            ImGui::OpenPopup("Compiling Scripts");
        }
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::BeginPopupModal("Compiling Scripts", NULL, flags))
    {
        ImGui::Text("Compiling Game Code...");
        ImGui::Spacing();
        
        float progress = CompilerState::progress.load();
        ImGui::ProgressBar(progress, ImVec2(400, 24));
        
        ImGui::Spacing();

        {
            std::lock_guard<std::mutex> lock(CompilerState::status_mutex);
            std::string display_text = CompilerState::status_text;
            if (display_text.length() > 60) display_text = display_text.substr(0, 57) + "...";
            ImGui::TextDisabled("%s", display_text.c_str());
        }

        if (!CompilerState::is_compiling.load())
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
