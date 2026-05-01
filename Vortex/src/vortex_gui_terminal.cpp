#include "vortex_gui.hpp"
#include "util/vortex_logs.hpp"
#include "util/vortex_global_vars.hpp"

void VortexGUI::draw_terminal()
{
    if (!show_terminal) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400), ImVec2(FLT_MAX, FLT_MAX));
    
    if (ImGui::Begin("Console Terminal"))
    {
        if (ImGui::Button("Clear"))
        {
            std::lock_guard<std::mutex> lock(VortexLog::s_LogMutex);
            VortexLog::s_LogBuffer.clear();
        }
        ImGui::SameLine();
        bool copy_to_clipboard = ImGui::Button("Copy to Clipboard");

        ImGui::Separator();

        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (copy_to_clipboard) ImGui::LogToClipboard();

        std::lock_guard<std::mutex> lock(VortexLog::s_LogMutex);
        for (const auto& entry : VortexLog::s_LogBuffer)
        {
            ImVec4 color;
            switch (entry.level)
            {
                case LogLevel::INFO:    color = ImVec4(0.5f, 0.9f, 0.5f, 1.0f); break; // Green
                case LogLevel::WARNING: color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); break; // Orange
                case LogLevel::ERROR:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break; // Red
                case LogLevel::FATAL:   color = ImVec4(0.3f, 0.8f, 1.0f, 1.0f); break; // Cyan
            }

            ImGui::TextDisabled("%s", entry.time.c_str());
            ImGui::SameLine();
            
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }

        if (copy_to_clipboard) ImGui::LogFinish();

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        
        ImGui::Separator();

        std::string st = "Vortex Engine " + GLOBAL::VORTEX_VERSION + " - Terminal Active";
        ImGui::TextDisabled("%s", st.c_str());
    }
    ImGui::End();
}
