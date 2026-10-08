#include "vortex_gui_ImGui.hpp"
#include "vortex_logs.hpp"
#include "vortex_global_vars.hpp"

#include <algorithm>
#include <cctype>

#ifndef DEFAULT_ICON_FA_TRASH
    #define DEFAULT_ICON_FA_TRASH "[X]"
#endif

VortexTerminalWindow::VortexTerminalWindow() : VortexImGuiWindow(getGuiWindowTypeName(GuiWindowTypes::TerminalWindow), false) {}

void VortexTerminalWindow::apply_window_constraints()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(1000, 400), ImVec2(FLT_MAX, FLT_MAX));
}

void VortexTerminalWindow::draw_content()
{
    if (ImGui::Button(DEFAULT_ICON_FA_TRASH " Clear"))
    {
        std::lock_guard<std::mutex> lock(VortexLog::s_LogMutex);
        VortexLog::s_LogBuffer.clear();
    }
    ImGui::SameLine();
    bool copy_to_clipboard = ImGui::Button("Copy");

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10.0f, 0.0f));
    ImGui::SameLine();

    ImGui::Checkbox("Info", &show_info); ImGui::SameLine();
    ImGui::Checkbox("Warnings", &show_warning); ImGui::SameLine();
    ImGui::Checkbox("Errors", &show_error); ImGui::SameLine();
    ImGui::Checkbox("Auto-Scroll", &auto_scroll);

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10.0f, 0.0f));
    ImGui::SameLine();

    ImGui::SetNextItemWidth(150.0f);
    ImGui::InputTextWithHint("##SubFilter", "Filter Subsystem...", subsystem_filter, sizeof(subsystem_filter));

    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

    static ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("LogTable", 3, table_flags, ImVec2(0, -footer_height_to_reserve)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Tags", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        if (copy_to_clipboard) ImGui::LogToClipboard();

        {
            std::lock_guard<std::mutex> lock(VortexLog::s_LogMutex);

            std::string filter_str = subsystem_filter;
            if (!filter_str.empty())
            {
                std::transform(filter_str.begin(), filter_str.end(), filter_str.begin(), ::tolower);
            }

            for (LogEntry& entry : VortexLog::s_LogBuffer)
            {
                if (!show_info    && entry.level == LogLevel::INFO)    continue;
                if (!show_warning && entry.level == LogLevel::WARNING) continue;
                if (!show_error   && entry.level == LogLevel::ERROR)   continue;
                if (!show_fatal   && entry.level == LogLevel::FATAL)   continue;

                if (!filter_str.empty())
                {
                    std::string tag_lower = entry.subsystemtag;
                    std::transform(tag_lower.begin(), tag_lower.end(), tag_lower.begin(), ::tolower);

                    if (tag_lower.find(filter_str) == std::string::npos)
                    {
                        continue;
                    }
                }

                render_log_entry(entry);
            }
        }

        if (copy_to_clipboard) ImGui::LogFinish();

        if (auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndTable();
    }

    ImGui::Separator();

    std::string st = GLOBAL::VORTEX_NAME + " " + GLOBAL::VORTEX_VERSION + " - Terminal Active";
    ImGui::TextDisabled("%s", st.c_str());
}

void VortexTerminalWindow::render_log_entry(LogEntry &entry)
{
    ImVec4 color;
    switch (entry.level)
    {
        case LogLevel::INFO:    color = ImVec4(0.5f, 0.9f, 0.5f, 1.0f); break; // Green
        case LogLevel::WARNING: color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f); break; // Orange
        case LogLevel::ERROR:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break; // Red
        case LogLevel::FATAL:   color = ImVec4(0.3f, 0.8f, 1.0f, 1.0f); break; // Cyan
        default:                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break; // White
    }

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::TextDisabled("%s", entry.time.c_str());

    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    if (entry.subsystemtag.empty())
    {
        ImGui::Text("[%s]", entry.tag.c_str());
    }
    else
    {
        ImGui::Text("[%s] [%s]", entry.tag.c_str(), entry.subsystemtag.c_str());
    }
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();
    ImGui::TextUnformatted(entry.message.c_str());
}
