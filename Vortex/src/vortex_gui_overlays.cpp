#include "vortex_gui.hpp"
#include "vortex_camera.hpp"

EngineStatsThreaded g_engine_stats;
void VortexGUI::engine_stats()
{
    if (!show_debug_gui) return;

    ImGui::SetNextWindowPos(ImVec2(10, 15), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        "Performance", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / fps);

    ImGui::Separator();

    EngineStats stats = g_engine_stats.get_stats();

    ImGui::ProgressBar(
        stats.cpu_usage / 100.0f, ImVec2(-1, 0),
        ("CPU: " + std::to_string((int)stats.cpu_usage) + "%").c_str()
    );

    ImGui::Text("RAM Usage: %.2f MB", stats.ram_usage);

    ImGui::Separator();

    if (stats.gpu_usage >= 0.0f)
    {
        ImGui::ProgressBar(
            stats.gpu_usage / 100.0f, ImVec2(-1, 0),
            ("GPU: " + std::to_string((int)stats.gpu_usage) + "%").c_str()
        );
    }
    else ImGui::Text("GPU Usage: N/A");

    if (stats.gpu_mem_total > 0)
    {
        ImGui::ProgressBar(
            stats.gpu_mem_usage / stats.gpu_mem_total, ImVec2(-1, 0),
            ("VRAM: " + std::to_string((int)stats.gpu_mem_usage) + " MB").c_str()
        );
    }

    ImGui::End();
}

void VortexGUI::camera_info(VortexCamera *camera)
{
    if (!camera || !show_debug_gui) return;

    ImGui::SetNextWindowPos(ImVec2(180, 10), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        "Camera", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("Position: %.1f, %.1f, %.1f", camera->position.x, camera->position.y, camera->position.z);
    ImGui::Text("Is Anchored: %s", camera->anchored ? "True" : "False");
    
    VortexGuiLambda::ClampedInputFloat("Speed", &camera->movement_speed, 0.5f, 1.0f, 0.0f, 100.0f);

    ImGui::End();
}
