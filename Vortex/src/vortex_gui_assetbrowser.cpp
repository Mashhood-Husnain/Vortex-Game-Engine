#include "vortex_gui.hpp"

#include <algorithm>
#include <cctype>

void VortexGUI::draw_asset_browser()
{
    if (!show_asset_browser) return;

    ImGui::Begin("Asset Browser");

    if (strlen(m_new_project_name) == 0)
    {
        ImGui::TextDisabled("No active project loaded");
        ImGui::End();
        return;
    }

    std::filesystem::path base_project_path = std::filesystem::path(VortexProject::SAVE_DIRECTORY) / m_new_project_name;

    static std::filesystem::path current_path = base_project_path;
    static std::string last_project = "";

    if (last_project != m_new_project_name)
    {
        current_path = base_project_path;
        last_project = m_new_project_name;
    }

    if (!std::filesystem::exists(current_path))
    {
        current_path = base_project_path;
    }

    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%s", current_path.string().c_str());
    ImGui::Separator();

    std::filesystem::path path_to_navigate_to = current_path;

    if (current_path != base_project_path)
    {
        if (ImGui::Button("<- Back", ImVec2(75, 25)) ||
            (VortexKeyboard::get_key("LEFTALT") &&
            VortexKeyboard::get_key_down("LEFT") &&
            app->is_mouse_visible())
        )
        {
            path_to_navigate_to = current_path.parent_path();
        }
        ImGui::Separator();
    }

    ImGui::Spacing();

    std::vector<std::filesystem::directory_entry> directory_entries;
    if (std::filesystem::exists(current_path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(current_path))
        {
            std::string filename = entry.path().filename().string();

            if (current_path == base_project_path)
            {
                if (filename != VortexProject::ASSET_DIR_AUDIO &&
                    filename != VortexProject::ASSET_DIR_MODELS &&
                    filename != VortexProject::ASSET_DIR_SCRIPTS)
                {
                    continue;
                }
            }

            directory_entries.push_back(entry);
        }
    }

    if (directory_entries.empty())
    {
        ImVec2 text_size = ImGui::CalcTextSize("Empty Folder");
        ImVec2 avail = ImGui::GetContentRegionAvail();

        ImGui::SetCursorPos(ImVec2(
            ImGui::GetCursorPosX() + (avail.x - text_size.x) * 0.5f,
            ImGui::GetCursorPosY() + (avail.y - text_size.y) * 0.5f
        ));
        ImGui::TextDisabled("Empty Folder");

        current_path = path_to_navigate_to;
        ImGui::End();
        return;
    }

    std::sort(directory_entries.begin(), directory_entries.end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
    {
        bool a_is_dir = a.is_directory();
        bool b_is_dir = b.is_directory();

        if (a_is_dir && !b_is_dir) return true;
        if (!a_is_dir && b_is_dir) return false;

        std::string a_str = a.path().filename().string();
        std::string b_str = b.path().filename().string();

        std::transform(a_str.begin(), a_str.end(), a_str.begin(), ::tolower);
        std::transform(b_str.begin(), b_str.end(), b_str.begin(), ::tolower);

        return a_str < b_str;
    });

    float padding = 32.0f;
    float thumbnail_size = 64.0f;
    float cell_size = thumbnail_size + padding;

    float panel_width = ImGui::GetContentRegionAvail().x;
    int column_count = (int)(panel_width / cell_size);
    if (column_count < 1) column_count = 1;

    ImGui::Columns(column_count, 0, false);

    for (const auto& entry : directory_entries)
    {
        std::string filename = entry.path().filename().string();

        ImGui::PushID(filename.c_str());

        if (entry.is_directory())
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.4f, 0.6f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.4f, 0.6f, 0.5f));

            if (ImGui::ImageButton(
                filename.c_str(),
                (ImTextureID)(intptr_t)folder_icon_tex,
                ImVec2(thumbnail_size, thumbnail_size)
            ))
            {
                path_to_navigate_to = current_path / entry.path().filename();
            }
            ImGui::PopStyleColor(3);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));

            ImGui::ImageButton(
                filename.c_str(),
                (ImTextureID)(intptr_t)file_icon_tex,
                ImVec2(thumbnail_size, thumbnail_size)
            );

            ImGui::PopStyleColor(3);
        }

        float text_width = ImGui::CalcTextSize(filename.c_str()).x;
        float total_item_width = thumbnail_size + (ImGui::GetStyle().FramePadding.x * 2.0f);
        float text_indent = (total_item_width - text_width) * 0.5f;

        if (text_indent > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + text_indent);

        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + cell_size - 8.0f);
        ImGui::TextWrapped("%s", filename.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);

    current_path = path_to_navigate_to;

    ImGui::End();
}
