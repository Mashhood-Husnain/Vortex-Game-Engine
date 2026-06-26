#include "vortex_gui.hpp"
#include "vortex_camera.hpp"

EngineStatsThreaded g_engine_stats;

void VortexGUI::engine_stats()
{
    if (!show_engine_stats) return;

    ImGui::Begin("Performance");

    if (ImGui::CollapsingHeader("FrameRate", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        static float display_vsync_fps = 60.0f;
        static float display_true_fps = 60.0f;
        static float display_frame_time = 16.6f;
        static float display_render_time = 1.0f;

        static float fps_update_timer = 0.0f;
        static int frame_count_this_interval = 0;

        static float accumulated_delta_time = 0.0f;
        static float accumulated_render_time = 0.0f;

        float current_delta = GLOBAL::deltaTime;
        float current_render = (app->get_true_fps() > 0.0f) ? (1.0f / app->get_true_fps()) : 0.0f;

        accumulated_delta_time += current_delta;
        accumulated_render_time += current_render;
        frame_count_this_interval++;
        fps_update_timer += current_delta;

        if (fps_update_timer >= 0.25f)
        {
            if (frame_count_this_interval > 0)
            {
                float avg_delta = accumulated_delta_time / frame_count_this_interval;
                float avg_render = accumulated_render_time / frame_count_this_interval;

                display_frame_time = avg_delta * 1000.0f;
                display_render_time = avg_render * 1000.0f;

                display_vsync_fps = (avg_delta > 0.0f) ? (1.0f / avg_delta) : 0.0f;
                display_true_fps = (avg_render > 0.0f) ? (1.0f / avg_render) : 0.0f;
            }

            fps_update_timer = 0.0f;
            frame_count_this_interval = 0;
            accumulated_delta_time = 0.0f;
            accumulated_render_time = 0.0f;
        }

        ImVec4 vsync_color = ImVec4(0.3f, 0.8f, 0.4f, 1.0f);
        if (display_vsync_fps < 55.0f) vsync_color = ImVec4(0.8f, 0.7f, 0.2f, 1.0f);
        if (display_vsync_fps < 30.0f) vsync_color = ImVec4(0.9f, 0.3f, 0.3f, 1.0f);

        const float TEXT_OFFSET = 120.0f;

        ImGui::TextDisabled("V-Sync FPS:");
        ImGui::SameLine(TEXT_OFFSET);
        ImGui::TextColored(vsync_color, "%.1f", display_vsync_fps);

        ImGui::TextDisabled("True FPS:");
        ImGui::SameLine(TEXT_OFFSET);
        ImGui::TextColored(ImVec4(0.5f, 0.9f, 1.0f, 1.0f), "%.1f", display_true_fps);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextDisabled("Frame Time:");
        ImGui::SameLine(TEXT_OFFSET);
        ImGui::Text("%.2f ms", display_frame_time);

        ImGui::TextDisabled("Render Time:");
        ImGui::SameLine(TEXT_OFFSET);
        ImGui::Text("%.2f ms", display_render_time);

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
    if (VortexCompiler::is_compiling())
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

        float progress = VortexCompiler::progress();
        ImGui::ProgressBar(progress, ImVec2(400, 24));

        ImGui::Spacing();

        {
            std::string display_text = VortexCompiler::status_text();
            if (display_text.length() > 60) display_text = display_text.substr(0, 57) + "...";
            ImGui::TextDisabled("%s", display_text.c_str());
        }

        if (!VortexCompiler::is_compiling())
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void VortexGUI::draw_stack_history_window()
{
    if (!show_stack_history_window) return;

    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300.0f, 400.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Action Stack History");

    const auto& undo_stack = ActionManager::get_undo_stack();
    const auto& redo_stack = ActionManager::get_redo_stack();

    ImGui::TextDisabled("History Limit: %zu / 50", undo_stack.size() + redo_stack.size());
    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    for (auto it = redo_stack.begin(); it != redo_stack.end(); ++it)
    {
        ImGui::Text("  %s %s", (*it)->get_name().c_str(), (*it)->get_details().c_str());
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::Text("-> Current State");
    ImGui::PopStyleColor();

    for (auto it = undo_stack.rbegin(); it != undo_stack.rend(); ++it)
    {
        ImGui::Text("  %s %s", (*it)->get_name().c_str(), (*it)->get_details().c_str());
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(0.0f);
    }

    ImGui::End();
}
