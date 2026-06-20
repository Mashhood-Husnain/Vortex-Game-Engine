#include "vortex_gui.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

GLuint VortexGUI::return_file_icon_tex(const std::string &ext_name)
{
    if (ext_name == ".cpp") return file_cpp_icon_tex;
    else if (ext_name == ".hpp") return file_hpp_icon_tex;
    else if (ext_name == ".mtl") return file_mtl_icon_tex;
    else if (ext_name == ".obj") return file_obj_icon_tex;
    else if (ext_name == ".png") return file_png_icon_tex;
    else if (ext_name == ".txt") return file_txt_icon_tex;
    else return file_icon_tex;
}

GLuint VortexGUI::return_folder_icon_tex(const std::filesystem::directory_entry &entry)
{
    try
    {
        if (std::filesystem::is_empty(entry.path())) return folder_icon_tex;
    }
    catch (const std::filesystem::filesystem_error&)
    {
        VORTEX_WARN("Couldn't determine wether folder is empty or not");
    }

    return folder_full_icon_tex;
}

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

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
            {
                std::string dragged_path_str = static_cast<const char*>(payload->Data);
                std::filesystem::path dragged_path(dragged_path_str);

                std::filesystem::path parent_dir = current_path.parent_path();
                std::filesystem::path new_destination = parent_dir / dragged_path.filename();

                try
                {
                    std::filesystem::rename(dragged_path, new_destination);
                    VORTEX_INFO("[ASSET BROWSER] Moved item UP to parent directory: ", new_destination.string());
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    VORTEX_ERROR("[ASSET BROWSER] Failed to move item up: ", e.what());
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Separator();
    }

    ImGui::Spacing();

    std::vector<std::filesystem::directory_entry> directory_entries;
    std::string settings_filename = std::string(m_new_project_name) + "_settings.json";
    if (std::filesystem::exists(current_path))
    {
        for (const auto& entry : std::filesystem::directory_iterator(current_path))
        {
            if (entry.path().extension() == ".vtx" || entry.path().filename().string() == settings_filename ||
                entry.path().filename().string() == "CMakeLists.txt" || entry.path().filename().string() == ".vscode")
            {
                continue;
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
    }
    else
    {
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
    }

    float padding = 32.0f;
    float thumbnail_size = 64.0f;
    float cell_size = thumbnail_size + padding;

    float panel_width = ImGui::GetContentRegionAvail().x;
    int column_count = (int)(panel_width / cell_size);
    if (column_count < 1) column_count = 1;

    ImGui::Columns(column_count, 0, false);

    static bool show_delete_modal = false;
    static char item_to_delete[512] = "";

    if (!directory_entries.empty())
    {
        for (const auto& entry : directory_entries)
        {
            std::string filename = entry.path().filename().string();
            std::string file_extname = entry.path().extension().string();
            std::string absolute_path = entry.path().string();

            ImGui::PushID(absolute_path.c_str());

            if (entry.is_directory())
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.4f, 0.6f, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.4f, 0.6f, 0.5f));

                if (ImGui::ImageButton(
                    filename.c_str(),
                    (ImTextureID)(intptr_t)return_folder_icon_tex(entry),
                    ImVec2(thumbnail_size, thumbnail_size)
                ))
                {
                    path_to_navigate_to = current_path / entry.path().filename();
                }
                ImGui::PopStyleColor(3);

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", absolute_path.c_str(), absolute_path.size() + 1);
                    ImGui::Text("Move %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
                    {
                        std::string dragged_path_str = static_cast<const char*>(payload->Data);
                        std::filesystem::path dragged_path(dragged_path_str);

                        if (dragged_path != entry.path())
                        {
                            std::filesystem::path new_destination = entry.path() / dragged_path.filename();
                            try
                            {
                                std::filesystem::rename(dragged_path, new_destination);
                                VORTEX_INFO("[ASSET BROWSER] Moved item to: ", new_destination.string());
                            }
                            catch (const std::filesystem::filesystem_error& e)
                            {
                                VORTEX_ERROR("[ASSET BROWSER] Failed to move item: ", e.what());
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 0.5f));

                ImGui::ImageButton(
                    filename.c_str(),
                    (ImTextureID)(intptr_t)return_file_icon_tex(file_extname),
                    ImVec2(thumbnail_size, thumbnail_size)
                );

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    open_file_in_viewer(entry.path().string());
                }

                ImGui::PopStyleColor(3);

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", absolute_path.c_str(), absolute_path.size() + 1);
                    ImGui::Text("Move %s", filename.c_str());
                    ImGui::EndDragDropSource();
                }
            }

            if (ImGui::BeginPopupContextItem(("Context##" + filename).c_str()))
            {
                if (entry.is_regular_file())
                {
                    if (ImGui::MenuItem("Open in External Editor"))
                    {
                        if (strlen(preferred_ide_path) > 0)
                        {
                            std::string cmd;
                            std::string file_path = entry.path().string();

                            #if defined(_WIN32) || defined(_WIN64)
                                cmd = "start \"\" \"" + std::string(preferred_ide_path) + "\" \"" + file_path + "\"";
                            #else
                                cmd = std::string(preferred_ide_path) + " \"" + file_path + "\" &";
                            #endif

                            int result = std::system(cmd.c_str());
                            if (result != 0)
                            {
                                VORTEX_WARN("[System] Failed to launch external editor. Check IDE path in settings.");
                            }
                        }
                    }
                    ImGui::Separator();
                }

                if (ImGui::MenuItem("Delete"))
                {
                    vortex_strncpy(item_to_delete, sizeof(item_to_delete), entry.path().string().c_str());
                    show_delete_modal = true;
                }

                ImGui::EndPopup();
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
    }

    ImGui::InvisibleButton("##AssetBrowserBackgroundDrop", ImGui::GetContentRegionAvail());
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
        {
            std::string dragged_path_str = static_cast<const char*>(payload->Data);
            std::filesystem::path dragged_path(dragged_path_str);

            if (dragged_path.parent_path() != current_path)
            {
                std::filesystem::path new_destination = current_path / dragged_path.filename();
                try
                {
                    std::filesystem::rename(dragged_path, new_destination);
                    VORTEX_INFO("[ASSET BROWSER] Moved item to current directory: ", new_destination.string());
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    VORTEX_ERROR("[ASSET BROWSER] Failed to move item: ", e.what());
                }
            }
        }
        ImGui::EndDragDropTarget();
    }

    current_path = path_to_navigate_to;

    if (ImGui::BeginPopupContextWindow("AssetBrowserContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Create New File"))
        {
            memset(new_item_name, 0, sizeof(new_item_name));
            pending_creation_path = current_path.string();
            show_create_file_modal = true;
        }
        if (ImGui::MenuItem("Create New Folder"))
        {
            memset(new_item_name, 0, sizeof(new_item_name));
            pending_creation_path = current_path.string();
            show_create_folder_modal = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Open Project in IDE"))
        {
            if (strlen(preferred_ide_path) > 0)
            {
                std::string scripts_dir = vortex_generatepath(
                    VortexProject::SAVE_DIRECTORY,
                    m_new_project_name,
                    VortexProject::ASSET_DIR_SCRIPTS
                );

                std::string absolute_dir = std::filesystem::absolute(scripts_dir).string();

                #if defined(_WIN32) || defined(_WIN64)
                    std::string cmd = "start \"\" \"" + std::string(preferred_ide_path) + "\" \"" + absolute_dir + "\"";
                #else
                    std::string cmd = std::string(preferred_ide_path) + " \"" + absolute_dir + "\" &";
                #endif

                if (std::system(cmd.c_str()) != 0) VORTEX_WARN("Failed to launch external editor.");
            }
            else
            {
                VORTEX_WARN("[Editor] No IDE path set in Edit -> Settings!");
            }
        }

        ImGui::EndPopup();
    }

    ImVec2 modal_center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(modal_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (show_create_file_modal) ImGui::OpenPopup("Create File");
    if (ImGui::BeginPopupModal("Create File", &show_create_file_modal, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter file name (e.g., script.cpp):");
        ImGui::InputText("##filename", new_item_name, IM_ARRAYSIZE(new_item_name));
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::string full_path = vortex_generatepath(pending_creation_path, new_item_name);
            std::ofstream new_file(full_path);
            new_file.close();
            show_create_file_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) show_create_file_modal = false;
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(modal_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (show_create_folder_modal) ImGui::OpenPopup("Create Folder");
    if (ImGui::BeginPopupModal("Create Folder", &show_create_folder_modal, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter folder name:");
        ImGui::InputText("##foldername", new_item_name, IM_ARRAYSIZE(new_item_name));
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            std::string full_path = vortex_generatepath(pending_creation_path, new_item_name);

            std::filesystem::create_directory(full_path);
            show_create_folder_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) show_create_folder_modal = false;
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(modal_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (show_delete_modal) ImGui::OpenPopup("Delete Asset");
    if (ImGui::BeginPopupModal("Delete Asset", &show_delete_modal, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to delete this item?");
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "This action cannot be undone!");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Yes, Delete", ImVec2(120, 0)))
        {
            try
            {
                std::filesystem::remove_all(item_to_delete);
                VORTEX_INFO("[ASSET BROWSER] Successfully deleted: ", item_to_delete);
            }
            catch (const std::filesystem::filesystem_error& e)
            {
                VORTEX_ERROR("[ASSET BROWSER ERROR] Failed to delete item: ", e.what());
            }

            show_delete_modal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) show_delete_modal = false;
        ImGui::EndPopup();
    }

    ImGui::End();
}
