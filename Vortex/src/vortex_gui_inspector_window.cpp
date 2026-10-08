#include "vortex_gui_ImGui.hpp"
#include "vortex_model.hpp"
#include "vortex_particlesystem.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_assetmanager.hpp"
#include "vortex_decal.hpp"

struct FolderNode
{
    std::vector<VortexModel*> models;
    std::unordered_map<std::string, FolderNode> subfolders;
};

VortexInspectorWindow::VortexInspectorWindow() : VortexImGuiWindow(getGuiWindowTypeName(GuiWindowTypes::InspectorWindow), true) {}

void VortexInspectorWindow::apply_window_constraints()
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));
}

void VortexInspectorWindow::draw_content()
{
    m_processed_models.clear();
    m_processed_ps.clear();

    static char new_folder_path[128] = "Scene/NewFolder";
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 100);
    ImGui::InputText("##NewFolder", new_folder_path, IM_ARRAYSIZE(new_folder_path));
    ImGui::SameLine();

    if (ImGui::Button("Create Path") && strlen(new_folder_path) > 0)
    {
        std::string path(new_folder_path);
        if (std::find(VortexGUI::explicit_empty_folders.begin(), VortexGUI::explicit_empty_folders.end(), path) == VortexGUI::explicit_empty_folders.end())
        {
            VortexGUI::explicit_empty_folders.push_back(path);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    FolderNode root;

    auto insert_into_tree = [&](const std::string& path, VortexModel* model)
    {
        std::stringstream ss(path);
        std::string part;
        FolderNode* current = &root;

        while (std::getline(ss, part, '/'))
        {
            if (part.empty()) continue;
            current = &(current->subfolders[part]);
        }

        if (model) current->models.push_back(model);
    };

    for (const std::string& empty_path : VortexGUI::explicit_empty_folders)
    {
        insert_into_tree(empty_path, nullptr);
    }

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        insert_into_tree(model->folder.empty() ? "Scene" : model->folder, model);
    }

    std::function<void(const std::string&, FolderNode&, std::string)> draw_node;
    draw_node = [&](const std::string& name, FolderNode& node, std::string current_path)
    {
        bool folder_open = VortexUI::Helpers::DrawComponentNode(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));

        if (ImGui::BeginPopupContextItem())
        {
            if (name == "Scene")
            {
                ImGui::TextDisabled("Default folder cannot be deleted.");
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                if (ImGui::Selectable("Delete Folder"))
                {
                    for (VortexModel* m : VortexObjectManager::active_models)
                    {
                        if (m->folder.find(current_path) == 0) m->folder = "Scene";
                    }

                    auto it = std::find(VortexGUI::explicit_empty_folders.begin(), VortexGUI::explicit_empty_folders.end(), current_path);
                    if (it != VortexGUI::explicit_empty_folders.end()) VortexGUI::explicit_empty_folders.erase(it);

                    ImGui::PopStyleColor();
                    ImGui::EndPopup();
                    if (folder_open) ImGui::TreePop();
                    return;
                }
                ImGui::PopStyleColor();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_NODE"))
            {
                VortexModel* dragged_model = *(VortexModel**)payload->Data;

                if (VortexObjectManager::selected_models.find(dragged_model) != VortexObjectManager::selected_models.end())
                {
                    for (VortexModel* m : VortexObjectManager::selected_models) m->folder = current_path;
                }
                else
                {
                    dragged_model->folder = current_path;
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (folder_open)
        {
            for (auto& [sub_name, sub_node] : node.subfolders)
            {
                std::string next_path = current_path.empty() ? sub_name : VortexUtils::vortex_generatepath(current_path, sub_name);
                draw_node(sub_name, sub_node, next_path);
            }

            for (VortexModel* model : node.models) inspect_model(model);

            if (node.subfolders.empty() && node.models.empty()) ImGui::TextDisabled("  (Empty)");

            ImGui::TreePop();
        }
    };

    for (auto& [root_name, root_node] : root.subfolders)
    {
        draw_node(root_name, root_node, root_name);
    }

    ImGui::Separator(); ImGui::Spacing();

    if (ImGui::CollapsingHeader("Particle Systems", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (ParticleSystem* ps : VortexObjectManager::active_particlesystems)
        {
            inspect_particle_system(ps);
        }
    }
}

void VortexInspectorWindow::inspect_model(VortexModel* model)
{
    if (!model) return;
    if (m_processed_models.find(model) != m_processed_models.end()) return;

    m_processed_models.insert(model);
    ImGuiTreeNodeFlags sub_header_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    std::string header_id = model->model_name + "###" + std::to_string((uintptr_t)model);
    bool is_currently_selected = (VortexObjectManager::selected_models.find(model) != VortexObjectManager::selected_models.end());

    bool is_header_open = VortexUI::Helpers::DrawHeader(header_id.c_str(), is_currently_selected);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        if (ImGui::GetIO().KeyCtrl)
        {
            if (is_currently_selected)
            {
                VortexObjectManager::selected_models.erase(model);
                model->is_selected = false;
            }
            else
            {
                VortexObjectManager::selected_models.insert(model);
                model->is_selected = true;
            }
        }
        else if(!is_currently_selected)
        {
            for (VortexModel* m : VortexObjectManager::selected_models) m->is_selected = false;
            VortexObjectManager::selected_models.clear();
            VortexObjectManager::selected_models.insert(model);
            model->is_selected = true;
        }
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::GetIO().KeyCtrl)
    {
        if (is_currently_selected && VortexObjectManager::selected_models.size() > 1)
        {
            ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
            {
                for (VortexModel* m : VortexObjectManager::selected_models) m->is_selected = false;
                VortexObjectManager::selected_models.clear();
                VortexObjectManager::selected_models.insert(model);
                model->is_selected = true;
            }
        }
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload("MODEL_NODE", &model, sizeof(VortexModel*));
        if (VortexObjectManager::selected_models.size() > 1 && is_currently_selected) ImGui::Text("Moving %zu selected items...", VortexObjectManager::selected_models.size());
        else ImGui::Text("Moving: %s", model->model_name.c_str());
        ImGui::EndDragDropSource();
    }

    if (is_header_open)
    {
        ImGui::PushID(model);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::Indent(12.0f);
        ImGui::Spacing();

        draw_model_info_panel(model, sub_header_flags);
        draw_model_components_panel(model, sub_header_flags);
        draw_model_transform_panel(model, sub_header_flags);
        draw_model_actions(model);

        ImGui::Unindent(12.0f);
        ImGui::PopStyleVar(2);
        ImGui::PopID();
        ImGui::Spacing();
    }
}

void VortexInspectorWindow::draw_model_info_panel(VortexModel* model, int flags)
{
    if (ImGui::TreeNodeEx("|__ Model Info", flags))
    {
        ImGui::Indent(10.0f); ImGui::Spacing();

        ImGui::TextDisabled("Object Name");
        char name_buffer[256];
        memset(name_buffer, 0, sizeof(name_buffer));
        snprintf(name_buffer, sizeof(name_buffer), "%s", model->model_name.c_str());

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##ObjectName", name_buffer, IM_ARRAYSIZE(name_buffer))) model->model_name = std::string(name_buffer);

        ImGui::Spacing();
        ImGui::TextDisabled("Source File:"); ImGui::SameLine(); ImGui::TextWrapped("%s", model->file_path.c_str());
        ImGui::TextDisabled("Sub-Objects: %zu", model->shared_data->objects.size());

        ImGui::Spacing();
        ImGui::TextDisabled("Albedo/Diffuse Texture");

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::BeginChild("TextureDropZone", ImVec2(0, 80), true);

        if (model->texture_id != 0)
        {
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 64) * 0.5f);
            ImGui::SetCursorPosY(8.0f);
            ImGui::Image((void*)(intptr_t)model->texture_id, ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImVec2 text_size = ImGui::CalcTextSize("Drag & Drop Image Here");
            ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - text_size.x) * 0.5f, (ImGui::GetWindowHeight() - text_size.y) * 0.5f));
            ImGui::TextDisabled("Drag & Drop Image Here");
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
            {
                std::string path_str((const char*)payload->Data);
                if (path_str.find(".png") != std::string::npos || path_str.find(".jpg") != std::string::npos)
                {
                    VortexAssetManager::release_texture(model->texture_path);
                    model->texture_id = VortexAssetManager::load_texture_raw(path_str);
                    model->texture_path = path_str;
                    VORTEX_INFO_TAG("GUI", "Assigned new texture: ", path_str);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (model->texture_id != 0)
        {
            ImGui::Spacing();
            ImGui::TextDisabled("Texture Tiling (UV Scale)");
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat2("##TexScale", &model->texture_scale.x, 0.1f, 0.1f, 100.0f, "%.2f");

            ImGui::Spacing();
            ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 110.0f);

            if (VortexUI::Helpers::DrawDangerButton("Remove Texture", ImVec2(110, 24)))
            {
                VortexAssetManager::release_texture(model->texture_path);
                model->texture_id = 0;
                model->texture_path = "";
                VORTEX_INFO_TAG("GUI", "Removed texture from model: ", model->model_name);
            }
        }

        ImGui::Spacing(); ImGui::Unindent(10.0f);
    }
    ImGui::Spacing();
}

void VortexInspectorWindow::draw_model_components_panel(VortexModel* model, int flags)
{
    VortexEditorContext &ctx = VortexImGuiManager::context;

    if (ImGui::TreeNodeEx("|__ Components & Scripts", flags))
    {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.3f, 0.4f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

        ImGui::BeginChild("ScriptDropZone", ImVec2(0, 50), true);
        ImVec2 text_size = ImGui::CalcTextSize("+ Drag & Drop Script Here");
        ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - text_size.x) * 0.5f, (ImGui::GetWindowHeight() - text_size.y) * 0.5f));
        ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "+ Drag & Drop Script Here");
        ImGui::EndChild();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
            {
                std::string path_str((const char*)payload->Data);
                if (path_str.find(".cpp") != std::string::npos || path_str.find(".hpp") != std::string::npos)
                {
                    std::string script_name = std::filesystem::path(path_str).stem().string();
                    VortexMonoBehaviour *new_script = ScriptRegistry::get().create(script_name);
                    if (new_script)
                    {
                        new_script->vortexGameObject = model;
                        new_script->vortexEngine = ctx.application;
                        new_script->vortexTransform = &model->transform;
                        model->add_behaviour(script_name, new_script);
                        VORTEX_INFO_TAG("GUI", "Attached script: ", script_name);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Spacing();

        if (model->rigidbody)
        {
            if (VortexUI::Helpers::DrawComponentNode("Vortex Rigidbody", flags, ImVec4(0.15f, 0.22f, 0.15f, 1.0f)))
            {
                ImGui::Indent(10.0f); ImGui::Spacing();
                ImGui::Checkbox("Is Kinematic", &model->rigidbody->is_kinematic);
                ImGui::Checkbox("Gravity", &model->rigidbody->gravity);
                if (model->rigidbody->gravity) ImGui::DragFloat("Gravity Value", &model->rigidbody->gravity_value, 0.1f);

                ImGui::Spacing(); ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
                if (VortexUI::Helpers::DrawDangerButton("Remove##RB", ImVec2(70, 0))) { delete model->rigidbody; model->rigidbody = nullptr; }

                ImGui::Spacing(); ImGui::Unindent(10.0f);
            }
        }

        if (!model->decals.empty())
        {
            if (VortexUI::Helpers::DrawComponentNode("Vortex Decals", flags, ImVec4(0.22f, 0.15f, 0.22f, 1.0f)))
            {
                ImGui::Indent(10.0f); ImGui::Spacing();
                int decal_to_delete = -1;

                for (size_t i = 0; i < model->decals.size(); i++)
                {
                    VortexDecal* decal = model->decals[i];
                    ImGui::PushID(static_cast<int>(i));

                    ImVec4 header_color = (g_active_decal_for_gizmo == decal) ? ImVec4(0.3f, 0.2f, 0.3f, 1.0f) : ImVec4(0.18f, 0.12f, 0.18f, 1.0f);
                    bool single_decal_open = VortexUI::Helpers::DrawComponentNode(("Decal " + std::to_string(i)).c_str(), flags, header_color);

                    if (ImGui::IsItemClicked()) g_active_decal_for_gizmo = decal;

                    if (single_decal_open)
                    {
                        ImGui::Indent(10.0f); ImGui::Spacing();

                        ImGui::Checkbox("Flip X", &decal->flip_x); ImGui::SameLine();
                        ImGui::Checkbox("Flip Y", &decal->flip_y);
                        ImGui::Spacing();
                        ImGui::TextDisabled("Decal Texture (Drag & Drop)");

                        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.12f, 1.0f));
                        ImVec2 drop_size(ImGui::GetContentRegionAvail().x, 60.0f);
                        ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                        ImGui::InvisibleButton("##decal_drop_target", drop_size);
                        ImGui::GetWindowDrawList()->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + drop_size.x, cursor_pos.y + drop_size.y), IM_COL32(25, 25, 30, 255), 8.0f);

                        if (decal->quad_model && decal->quad_model->texture_id != 0)
                        {
                            ImVec2 img_pos(cursor_pos.x + (drop_size.x - 48.0f) * 0.5f, cursor_pos.y + 6.0f);
                            ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)decal->quad_model->texture_id, img_pos, ImVec2(img_pos.x + 48, img_pos.y + 48), ImVec2(0, 1), ImVec2(1, 0));
                        }
                        else
                        {
                            const char* text = "Drop Transparent PNG Here";
                            ImVec2 t_size = ImGui::CalcTextSize(text);
                            ImVec2 t_pos(cursor_pos.x + (drop_size.x - t_size.x) * 0.5f, cursor_pos.y + (drop_size.y - t_size.y) * 0.5f);
                            ImGui::GetWindowDrawList()->AddText(t_pos, IM_COL32(128, 128, 128, 255), text);
                        }
                        ImGui::PopStyleColor();

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM"))
                            {
                                std::string path_str((const char*)payload->Data);
                                if (path_str.find(".png") != std::string::npos || path_str.find(".jpg") != std::string::npos) decal->set_texture(path_str);
                            }
                            ImGui::EndDragDropTarget();
                        }

                        ImGui::Spacing(); ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
                        if (VortexUI::Helpers::DrawDangerButton("Remove##Decal", ImVec2(70, 0)))
                        {
                            if (g_active_decal_for_gizmo == decal) g_active_decal_for_gizmo = nullptr;

                            ActionDeleteDecal* delete_command = new ActionDeleteDecal(model, decal);
                            delete_command->redo();
                            ActionManager::push_action(delete_command);
                        }

                        ImGui::Spacing(); ImGui::Unindent(10.0f);
                    }
                    ImGui::PopID();
                }
                ImGui::Spacing(); ImGui::Unindent(10.0f);
            }
        }

        if (model->light)
        {
            if (VortexUI::Helpers::DrawComponentNode("Vortex Light", flags, ImVec4(0.22f, 0.22f, 0.15f, 1.0f)))
            {
                ImGui::Indent(10.0f); ImGui::Spacing();
                ImGui::ColorEdit3("Light Color", &model->light->color.x);
                ImGui::DragFloat("Intensity", &model->light->intensity, 0.05f, 0.0f, 100.0f);
                ImGui::DragFloat("Ambient Strength", &model->light->ambient_strength, 0.01f, 0.0f, 1.0f);

                ImGui::Spacing(); ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
                if (VortexUI::Helpers::DrawDangerButton("Remove##Light", ImVec2(70, 0)))
                {
                    delete model->light;
                    model->light = nullptr;
                }

                ImGui::Spacing(); ImGui::Unindent(10.0f);
            }
        }

        if (model->script_names.empty() && !model->rigidbody && !model->light) ImGui::TextDisabled("No components attached.");
        else
        {
            int script_to_delete = -1;
            for (size_t i = 0; i < model->script_names.size(); i++)
            {
                ImGui::PushID(static_cast<int>(i));
                std::string script_name = model->script_names[i];
                VortexMonoBehaviour *script = model->behaviours[i];

                bool script_open = VortexUI::Helpers::DrawComponentNode(script_name.c_str(), flags, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));

                if (script_open)
                {
                    ImGui::Indent(10.0f); ImGui::Spacing();

                    for (auto &var : script->exposed_variables)
                    {
                        if (!var.show_in_editor) continue;
                        if (var.type == ScriptVarType::INT) ImGui::DragInt(var.name.c_str(), (int*)var.data_ptr);
                        else if (var.type == ScriptVarType::FLOAT) ImGui::DragFloat(var.name.c_str(), (float*)var.data_ptr, 0.1f);
                        else if (var.type == ScriptVarType::BOOL) ImGui::Checkbox(var.name.c_str(), (bool*)var.data_ptr);
                        else if (var.type == ScriptVarType::VEC3) ImGui::DragFloat3(var.name.c_str(), (float*)var.data_ptr, 0.1f);
                        else if (var.type == ScriptVarType::STRING)
                        {
                            std::string *str_ptr = (std::string*)var.data_ptr;
                            char buffer[256];
                            VortexUtils::vortex_strncpy(buffer, sizeof(buffer), str_ptr->c_str());
                            if (ImGui::InputText(var.name.c_str(), buffer, IM_ARRAYSIZE(buffer))) *str_ptr = std::string(buffer);
                        }
                    }

                    ImGui::Spacing(); ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
                    if (VortexUI::Helpers::DrawDangerButton("Remove", ImVec2(70, 0))) script_to_delete = static_cast<int>(i);

                    ImGui::Spacing(); ImGui::Unindent(10.0f);
                }
                ImGui::PopID();
            }

            if (script_to_delete != -1)
            {
                delete model->behaviours[script_to_delete];
                model->behaviours.erase(model->behaviours.begin() + script_to_delete);
                model->script_names.erase(model->script_names.begin() + script_to_delete);
            }
        }

        ImGui::Spacing();
        std::vector<std::string> available_scripts = ScriptRegistry::get().get_available_scripts();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##AddComponent", "Add Component..."))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.9f, 0.5f, 1.0f));
            ImGui::Selectable("Native Components", false, ImGuiSelectableFlags_Disabled);
            ImGui::PopStyleColor();

            if (model->get_componant<VortexRigidbody>() == nullptr && ImGui::Selectable("Vortex Rigidbody"))
            {
                model->rigidbody = new VortexRigidbody();
                model->rigidbody->vortexGameObject = model;
                model->rigidbody->vortexTransform = &model->transform;
            }
            if (model->get_componant<VortexLight>() == nullptr && ImGui::Selectable("Vortex Light"))
            {
                model->light = new VortexLight();
                model->light->vortexGameObject = model;
                model->light->vortexTransform = &model->transform;
            }
            if (ImGui::Selectable("Vortex Decal"))
            {
                VortexDecal* new_decal = new VortexDecal();
                float z_extent = model->shared_data->collider.max.z;
                new_decal->position = glm::vec3(0.0f, 0.0f, z_extent + 0.01f);
                new_decal->orientation = glm::quat(glm::radians(glm::vec3(90.0f, 0.0f, 0.0f)));
                new_decal->vortexGameObject = model;
                new_decal->vortexEngine = ctx.application;
                new_decal->vortexTransform = &model->transform;
                new_decal->on_start();

                ActionCreateDecal* create_command = new ActionCreateDecal(model, new_decal);
                create_command->redo();
                ActionManager::push_action(create_command);
            }

            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.7f, 1.0f, 1.0f));
            ImGui::Selectable("Game Scripts (Hot-Reloadable)", false, ImGuiSelectableFlags_Disabled);
            ImGui::PopStyleColor();

            for (const std::string &script_name : available_scripts)
            {
                if (ImGui::Selectable(script_name.c_str()))
                {
                    VortexMonoBehaviour *new_script = ScriptRegistry::get().create(script_name);
                    if (new_script)
                    {
                        new_script->vortexGameObject = model;
                        new_script->vortexEngine = ctx.application;
                        new_script->vortexTransform = &model->transform;
                        model->add_behaviour(script_name, new_script);
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing(); ImGui::Unindent(10.0f);
    }
    ImGui::Spacing();
}

void VortexInspectorWindow::draw_model_transform_panel(VortexModel* model, int flags)
{
    if (ImGui::TreeNodeEx("|__ Physics & Transform", flags))
    {
        ImGui::Indent(10.0f); ImGui::Spacing();

        ImGui::Checkbox("Show Physics Collider", &model->show_collider);
        if (model->show_collider)
        {
            ImGui::Indent(10.0f); ImGui::Spacing();
            ImGui::DragFloat3("Collider Bounds", &model->collider_scale.x, 0.01f, 0.01f, 10.0f);

            float collider_uniform_scale = model->collider_scale.x;
            if (ImGui::SliderFloat("Uniform Bounds", &collider_uniform_scale, 0.001f, 10.0f)) model->collider_scale = glm::vec3(collider_uniform_scale);

            ImGui::Spacing();
            if (ImGui::Button("Reset Collider Size", ImVec2(-1, 24))) model->collider_scale = glm::vec3(1.0f);

            ImGui::Spacing(); ImGui::Unindent(10.0f);
        }

        ImGui::Spacing();
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 6));

        if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            VortexUI::Helpers::DrawTransformRow("Position", "##pos", &model->transform.position.x);
            VortexUI::Helpers::DrawTransformRow("Scale", "##scale", &model->transform.scale.x, 0.01f, 0.001f, 10.0f);
            VortexUI::Helpers::DrawTransformRow("Rotation", "##rot", &model->transform.orientation.x);

            ImGui::EndTable();
        }
        ImGui::PopStyleVar(2);

        ImGui::Spacing();
        float uniform_scale = model->transform.scale.x;
        if (ImGui::SliderFloat("Uniform Scale", &uniform_scale, 0.001f, 0.0f)) model->transform.scale = glm::vec3(uniform_scale);

        ImGui::Spacing();
        if (ImGui::Button("Reset Transform", ImVec2(-1, 32)))
        {
            model->transform.position = glm::vec3(0.0f);
            model->transform.scale = glm::vec3(1.0f);
            model->transform.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        ImGui::Spacing(); ImGui::Unindent(10.0f);
    }
    ImGui::Spacing();
}

void VortexInspectorWindow::draw_model_actions(VortexModel* model)
{
    if (VortexUI::Helpers::DrawActionButton("Duplicate Model", ImVec2(-1, 32)))
    {
        VortexModel* cloned_model = model->clone();

        ActionCreate* create_command = new ActionCreate(cloned_model);
        create_command->redo();
        ActionManager::push_action(create_command);

        VortexObjectManager::active_models.push_back(cloned_model);

        for (auto* m : VortexObjectManager::selected_models) m->is_selected = false;
        VortexObjectManager::selected_models.clear();

        cloned_model->is_selected = true;
        VortexObjectManager::selected_models.insert(cloned_model);
    }

    ImGui::Spacing();

    if (VortexUI::Helpers::DrawDangerButton("Delete Model", ImVec2(-1, 32)))
    {
        model->is_selected = false;
        VortexObjectManager::selected_models.erase(model);

        ActionDelete *delete_command = new ActionDelete(model);
        delete_command->redo();
        ActionManager::push_action(delete_command);
    }
}

void VortexInspectorWindow::inspect_particle_system(ParticleSystem *ps)
{
    if (!ps) return;
    if (m_processed_ps.find(ps) != m_processed_ps.end()) return;

    m_processed_ps.insert(ps);
    ImGuiTreeNodeFlags sub_header_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    std::string ps_header_id = "Particle System (" + ps->name +  ")##" + std::to_string((uintptr_t)ps);

    if (ImGui::CollapsingHeader(ps_header_id.c_str()))
    {
        ImGui::PushID(ps);
        ImGui::Indent(12.0f); ImGui::Spacing();

        if (ImGui::TreeNodeEx("|__ Emitters", sub_header_flags))
        {
            ImGui::Indent(10.0f); ImGui::Spacing();
            for (auto& [name, settings] : ps->emitter_registry)
            {
                ImGui::PushID(name.c_str());
                ImGui::Checkbox(name.c_str(), &settings.enabled);

                if (settings.enabled)
                {
                    ImGui::Indent(10.0f); ImGui::Spacing();

                    ImGui::TextDisabled("Spawning & Transform");
                    ImGui::DragFloat3("Position", &settings.position.x, 0.1f);
                    VortexUI::Lambda::ClampedInputInt("Spawn Rate", &settings.spawn_rate, 100, 500, 0, ps->max_particles);
                    VortexUI::Lambda::ClampedInputFloat("Size", &settings.size, 0.1f, 1.0f, 0.05f, 5.0f);
                    VortexUI::Lambda::ClampedInputFloat("Life", &settings.life, 0.5f, 1.0f, 0.1f, 10.0f);

                    const char *behaviour_options[] = {"Grow", "Shrink", "None"};
                    int current_behaviour = static_cast<int>(settings.behaviour);
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    if (ImGui::Combo("##Behaviour", &current_behaviour, behaviour_options, IM_ARRAYSIZE(behaviour_options)))
                    {
                        settings.behaviour = static_cast<ParticleBehaviour>(current_behaviour);
                    }
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    ImGui::TextDisabled("Visuals");
                    ImGui::ColorEdit4("Start Color", glm::value_ptr(settings.start_color));
                    ImGui::ColorEdit4("End Color", glm::value_ptr(settings.end_color));

                    int tex_id = static_cast<int>(settings.texture_id);
                    if (ImGui::InputInt("Texture ID (0 = Default)", &tex_id))
                    {
                        settings.texture_id = static_cast<GLuint>(std::max(0, tex_id));
                    }
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

                    ImGui::TextDisabled("Physics & Forces");
                    VortexUI::Lambda::ClampedInputFloat("Gravity", &settings.gravity, 0.2f, 0.5f, -2.0f, 2.0f);
                    ImGui::SliderFloat("Drag", &settings.drag, 0.0f, 5.0f);
                    ImGui::SliderFloat("Elasticity (Bounce)", &settings.elasticity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Friction", &settings.friction, 0.0f, 1.0f);
                    ImGui::SliderFloat("Turbulence", &settings.turbulence, 0.0f, 5.0f);

                    ImGui::Spacing();
                    if (ImGui::TreeNodeEx("Point Gravity"))
                    {
                        ImGui::Checkbox("Use Point Gravity", &settings.use_point_gravity);
                        if (settings.use_point_gravity)
                        {
                            ImGui::DragFloat3("Target Pos", glm::value_ptr(settings.gravity_point), 0.1f);
                            VortexUI::Lambda::ClampedInputFloat("Pull Strength", &settings.point_gravity_strength, 0.1f, 0.0f, 0.1f, 1.0f);
                        }
                        ImGui::TreePop();
                    }

                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                    if (ImGui::Button("Trigger Burst (100)", ImVec2(-1, 24)))
                    {
                        ps->burst(name, 100);
                    }

                    ImGui::Spacing(); ImGui::Unindent(10.0f);
                }
                ImGui::PopID(); ImGui::Spacing();
            }
            ImGui::Unindent(10.0f);
        }

        ImGui::Spacing();

        if (ImGui::TreeNodeEx("|__ Stats", sub_header_flags))
        {
            ImGui::Indent(10.0f); ImGui::Spacing();
            ImGui::TextDisabled("Total Particles:"); ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 60.0f); ImGui::Text("%d", (int)ps->max_particles);
            ImGui::TextDisabled("Active Particles:"); ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 60.0f); ImGui::Text("%d", ps->active_count);
            ImGui::Spacing(); ImGui::Unindent(10.0f);
        }

        ImGui::Spacing();

        if (VortexUI::Helpers::DrawDangerButton("Delete Particle System", ImVec2(-1, 32))) ps->should_destroy = true;

        ImGui::Unindent(12.0f);
        ImGui::PopID(); ImGui::Spacing();
    }
}
