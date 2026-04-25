#include "vortex_gui.hpp"
#include "vortex_camera.hpp"

EngineStatsThreaded g_engine_stats;

void VortexGUI::engine_stats()
{
    if (!show_debug_gui) return;

    ImGui::Begin("Performance");

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
    ImGui::Separator();
    ImGui::Spacing();

    EngineStats stats = g_engine_stats.get_stats();

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.75f, 0.35f, 0.15f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));

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
    ImGui::Separator();
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
    std::string vendor_text = "VENDOR: " + _vendor_;
    std::string renderer_text = "RENDERER: " + _renderer_;
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", vendor_text.c_str());
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", renderer_text.c_str());

    ImGui::PopStyleColor(2);
    ImGui::Spacing();

    ImGui::End();
}

void VortexGUI::camera_info(VortexCamera *camera)
{
    if (!camera || !show_debug_gui) return;

    ImGui::Begin("Camera");

    ImGui::TextDisabled("Position");
    ImGui::Text("X: %.1f  Y: %.1f  Z: %.1f", camera->position.x, camera->position.y, camera->position.z);
    
    ImGui::Spacing();
    
    ImGui::TextDisabled("Status:"); 
    ImGui::SameLine(60.0f);
    
    if (camera->anchored) 
    {
        ImGui::TextColored(ImVec4(0.85f, 0.45f, 0.20f, 1.0f), "[ Anchored ]");
    }
    else 
    {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.5f, 1.0f), "[ Free Roam ]");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::TextDisabled("Fly Speed");
    ImGui::SetNextItemWidth(-FLT_MIN);
    VortexGuiLambda::ClampedInputFloat("##Speed", &camera->movement_speed, 0.5f, 1.0f, 0.0f, 100.0f);

    ImGui::Spacing();
    ImGui::End();
}
