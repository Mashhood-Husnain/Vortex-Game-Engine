#include "vortex_gui.hpp"

#include <json.hpp>

void VortexGUI::draw_settings_window()
{
    if (!show_settings_window) return;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("Engine Settings", &show_settings_window, flags))
    {
        float window_width = ImGui::GetContentRegionAvail().x;
        float left_pane_width = window_width * 0.30f;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
        ImGui::BeginChild("SettingsCategories", ImVec2(left_pane_width, 0), true);

        static int selected_category = 0;
        if (ImGui::Selectable("General", selected_category == 0)) selected_category = 0;
        if (ImGui::Selectable("External Tools", selected_category == 1)) selected_category = 1;

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::BeginChild("SettingsContent", ImVec2(0, 0), true);

        if (selected_category == 0)
        {
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "General Settings");
            ImGui::Separator();
            ImGui::TextDisabled("More engine settings will be added");
        }
        else if (selected_category == 1)
        {
            ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "External Tools");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Preferred IDE / Text Editor Path");
            ImGui::TextDisabled("Provide absolute path for IDE / Text Editor.");

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##IDEPath", preferred_ide_path, IM_ARRAYSIZE(preferred_ide_path));
        }

        ImGui::EndChild();
    }
    ImGui::End();
}
