#include "vortex_gui.hpp"
#include "vortex_model.hpp"
#include "vortex_rigidbody.hpp"
#include "vortex_particlesystem.hpp"

void VortexGUI::begin_scene_inspector()
{
    if (!show_gui) return;

    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(FLT_MAX, FLT_MAX));

    ImGui::Begin("Scene Inspector");

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        inspector_info(model, nullptr);
    }

    for (ParticleSystem* ps : VortexObjectManager::active_particlesystems)
    {
        inspector_info(nullptr, ps);
    }

    bool is_collapsed = ImGui::IsWindowCollapsed();
    if (!is_collapsed && m_scene_was_collapsed)
    {
        m_force_creator_collapse = true;
    }
    m_scene_was_collapsed = is_collapsed;

    m_scene_pos = ImGui::GetWindowPos();
    m_scene_size = ImGui::GetWindowSize();
}

void VortexGUI::inspector_info(VortexModel* model, ParticleSystem *ps)
{
    if (!show_gui || (!model && !ps)) return;

    if (model)
    {
        if (m_processed_models.find(model) != m_processed_models.end()) return;

        m_processed_models.insert(model);
        
        std::string header_id = model->model_name + "###" + std::to_string((uintptr_t)model);

        bool is_currently_selected = (m_selected_model == model);
        
        if (is_currently_selected) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once); 
            
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.85f, 0.40f, 0.15f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.95f, 0.50f, 0.20f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.00f, 0.60f, 0.30f, 1.00f));
        }

        bool is_header_open = ImGui::CollapsingHeader(header_id.c_str());

        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_selected_model = model;            
            model->is_selected = true; 
        }

        if (is_currently_selected)
        {
            ImGui::PopStyleColor(3); 
        }

        if (is_header_open)
        {
            ImGui::PushID(model);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            ImGui::Spacing();
            ImGui::SeparatorText("Model Info");
            ImGui::Spacing();

            ImGui::TextDisabled("Object Name");
            char name_buffer[256];
            memset(name_buffer, 0, sizeof(name_buffer));
            snprintf(name_buffer, sizeof(name_buffer), "%s", model->model_name.c_str());

            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputText("##ObjectName", name_buffer, IM_ARRAYSIZE(name_buffer)))
            {
                model->model_name = std::string(name_buffer);
            }
            
            ImGui::Spacing();
            ImGui::TextDisabled("Source File:");
            ImGui::SameLine();
            ImGui::TextWrapped("%s", model->file_path.c_str());
            ImGui::TextDisabled("Sub-Objects: %zu", model->shared_data->objects.size());

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::SeparatorText("Attached Scripts");
            ImGui::Spacing();

            if (model->script_names.empty())
            {
                ImGui::TextDisabled("  No scripts attached.");
            }
            else
            {
                int script_to_delete = -1;

                for (size_t i = 0; i < model->script_names.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(i));

                    std::string script_name = model->script_names[i];
                    VortexMonoBehaviour *script = model->behaviours[i];

                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
                    bool script_open = ImGui::CollapsingHeader(script_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                    ImGui::PopStyleColor();

                    if (script_open)
                    {
                        ImGui::Indent();
                        ImGui::Spacing();

                        VortexRigidbody* rigid_body = model->get_componant<VortexRigidbody>();
                        if (rigid_body)
                        {
                            ImGui::Checkbox("Is Kinematic", &rigid_body->is_kinematic);
                            ImGui::Checkbox("Gravity", &rigid_body->gravity);
                            if (rigid_body->gravity)
                            {
                                ImGui::DragFloat("Gravity Value", &rigid_body->gravity_value, 0.1f);
                            }
                            ImGui::Spacing();
                        }

                        for (auto &var : script->exposed_variables)
                        {
                            if (!var.show_in_editor) continue;

                            if (var.type == ScriptVarType::INT)
                            {
                                ImGui::DragInt(var.name.c_str(), (int*)var.data_ptr);
                            }
                            else if (var.type == ScriptVarType::FLOAT)
                            {
                                ImGui::DragFloat(var.name.c_str(), (float*)var.data_ptr, 0.1f);
                            }
                            else if (var.type == ScriptVarType::BOOL)
                            {
                                ImGui::Checkbox(var.name.c_str(), (bool*)var.data_ptr);
                            }
                            else if (var.type == ScriptVarType::VEC3)
                            {
                                ImGui::DragFloat3(var.name.c_str(), (float*)var.data_ptr, 0.1f);
                            }
                            else if (var.type == ScriptVarType::STRING)
                            {
                                std::string *str_ptr = (std::string*)var.data_ptr;
                                char buffer[256];
                                strncpy(buffer, str_ptr->c_str(), sizeof(buffer));
                                if (ImGui::InputText(var.name.c_str(), buffer, IM_ARRAYSIZE(buffer)))
                                {
                                    *str_ptr = std::string(buffer);
                                }
                            }
                        }
                        
                        ImGui::Spacing();
                        ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMax().x - 70.0f);
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.15f, 0.15f, 0.8f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
                        if (ImGui::Button("Remove", ImVec2(70, 0)))
                        {
                            script_to_delete = static_cast<int>(i);
                        }
                        ImGui::PopStyleColor(2);
                        
                        ImGui::Spacing();
                        ImGui::Unindent();
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
            std::vector<std::string> available_scripts = ScriptRegistry::get().get_avaialble_scripts();
            
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Add Script", "Add Component..."))
            {
                for (const std::string &script_name : available_scripts)
                {
                    if (ImGui::Selectable(script_name.c_str()))
                    {
                        VortexMonoBehaviour *new_script = ScriptRegistry::get().create(script_name);
                        if (new_script)
                        {
                            model->add_behaviour(script_name, new_script);
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::SeparatorText("Physics & Transform");
            ImGui::Spacing();
            
            ImGui::Checkbox("Show Physics Collider", &model->show_collider);

            if (model->show_collider)
            {
                ImGui::Indent();
                ImGui::Spacing();
                
                ImGui::DragFloat3("Collider Bounds", &model->collider_scale.x, 0.01f, 0.01f, 10.0f);
                
                float collider_uniform_scale = model->collider_scale.x;
                if (ImGui::SliderFloat("Uniform Bounds", &collider_uniform_scale, 0.001f, 10.0f))
                {
                    model->collider_scale = glm::vec3(collider_uniform_scale);
                }

                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.32f, 1.0f));
                if (ImGui::Button("Reset Collider Size", ImVec2(-1, 24)))
                {
                    model->collider_scale = glm::vec3(1.0f);
                }
                ImGui::PopStyleColor(3);

                ImGui::Spacing();
                ImGui::Unindent();
            }

            ImGui::Spacing();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 6));

            if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("Position");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);            
                ImGui::DragFloat3("##pos", &model->transform.position.x, 0.1f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("Scale");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat3("##scale", &model->transform.scale.x, 0.01f, 0.001f, 10.0f);

                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextDisabled("Rotation");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat3("##rot", &model->transform.orientation.x, 0.1f);

                ImGui::EndTable();
            }
            ImGui::PopStyleVar();

            ImGui::Spacing();

            float uniform_scale = model->transform.scale.x;
            if (ImGui::SliderFloat("Uniform Scale", &uniform_scale, 0.001f, 10.0f))
            {
                model->transform.scale = glm::vec3(uniform_scale);
            }

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.22f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.32f, 1.0f));
            if (ImGui::Button("Reset Transform", ImVec2(-1, 32)))
            {
                model->transform.position = glm::vec3(0.0f);
                model->transform.scale = glm::vec3(1.0f);
                model->transform.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            }
            ImGui::PopStyleColor(3);

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.25f, 0.25f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.30f, 0.30f, 1.00f));
            if (ImGui::Button("Delete Model", ImVec2(-1, 32)))
            {
                model->should_destroy = true;
                model->is_selected = false;
                m_selected_model = nullptr;
            }
            ImGui::PopStyleColor(3);

            ImGui::PopStyleVar(3);
            ImGui::PopID();
            ImGui::Spacing();
        }
    }

    if (ps)
    {
        if (m_processed_ps.find(ps) != m_processed_ps.end()) return;
        m_processed_ps.insert(ps);

        std::string ps_header_id = "Particle System (" + ps->name +  ")##" + std::to_string((uintptr_t)ps);

        if (ImGui::CollapsingHeader(ps_header_id.c_str()))
        {
            ImGui::PushID(ps);
            
            ImGui::Spacing();
            ImGui::SeparatorText("Emitters");
            ImGui::Spacing();
            
            for (auto& [name, settings] : ps->emitter_registry)
            {
                ImGui::PushID(name.c_str());
                ImGui::Checkbox(name.c_str(), &settings.enabled);

                if (settings.enabled)
                {
                    ImGui::Indent();
                    ImGui::Spacing();
                    
                    ImGui::DragFloat3("Position", &settings.position.x, 0.1f);

                    VortexGuiLambda::ClampedInputInt("Spawn Rate", &settings.spawn_rate, 100, 500, 0, ps->max_particles);
                    VortexGuiLambda::ClampedInputFloat("Size", &settings.size, 0.1f, 1.0f, 0.05f, 5.0f);
                    VortexGuiLambda::ClampedInputFloat("Life", &settings.life, 0.5f, 1.0f, 0.1f, 10.0f);
                    VortexGuiLambda::ClampedInputFloat("Gravity", &settings.gravity, 0.2f, 0.5f, -2.0f, 2.0f);

                    ImGui::SliderFloat("Drag", &settings.drag, 0.0f, 5.0f);
                    ImGui::SliderFloat("Elasticity", &settings.elasticity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Friction", &settings.friction, 0.0f, 1.0f);

                    const char *behaviour_options[] = {"Grow", "Shrink", "None"};
                    int current_behaviour = static_cast<int>(settings.behaviour);

                    ImGui::SetNextItemWidth(-FLT_MIN);
                    if (ImGui::Combo("##Behaviour", &current_behaviour, behaviour_options, IM_ARRAYSIZE(behaviour_options)))
                    {
                        settings.behaviour = static_cast<ParticleBehaviour>(current_behaviour);
                    }

                    ImGui::Spacing();
                    ImGui::ColorEdit4("Color", glm::value_ptr(settings.color));
                    
                    ImGui::Spacing();
                    ImGui::SeparatorText("Point Gravity");
                    ImGui::Checkbox("Use Point Gravity", &settings.use_point_gravity);

                    if (settings.use_point_gravity)
                    {
                        ImGui::DragFloat3("Target Pos", glm::value_ptr(settings.gravity_point), 0.1f);
                        VortexGuiLambda::ClampedInputFloat("Pull Strength", &settings.point_gravity_strength, 0.1f, 0.0f, 0.1f, 1.0f);
                    }

                    ImGui::Spacing();
                    ImGui::Unindent();
                }

                ImGui::PopID();
                ImGui::Spacing();
            }

            ImGui::SeparatorText("Stats");
            ImGui::Spacing();
            
            ImGui::TextDisabled("Total Particles:");
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 60.0f);
            ImGui::Text("%d", (int)ps->max_particles);
            
            ImGui::TextDisabled("Active Particles:");
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 60.0f);
            ImGui::Text("%d", ps->active_count);

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.25f, 0.25f, 1.00f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.30f, 0.30f, 1.00f));
            if (ImGui::Button("Delete Particle System", ImVec2(-1, 32)))
            {
                ps->should_destroy = true;
            }
            ImGui::PopStyleColor(3);

            ImGui::PopID();
            ImGui::Spacing();
        }
    }
}

void VortexGUI::end_scene_inspector()
{
    if (!show_gui) return;

    ImGui::End();
}
