#include "vortex_gui.hpp"
#include "vortex_application.hpp"
#include "vortex_physics.hpp"

void VortexGUI::draw_editor_render_viewport(VortexCamera *camera)
{
    if (!show_render_scene_viewport) return;

    // TODO: automatically take up all available space (when scene view is not active)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (GLOBAL::current_engine_state == GLOBAL::EngineState::PLAY)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                        ImGuiWindowFlags_NoNavFocus;

        is_render_view_visible = ImGui::Begin("Play Mode", nullptr, window_flags);
    }
    else
    {
        is_render_view_visible = ImGui::Begin("Render View");
    }

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    render_viewport_width = viewportPanelSize.x;
    render_viewport_height = viewportPanelSize.y;

    ImGui::Image((void*)(intptr_t)render_scene_texture, ImVec2(render_viewport_width, render_viewport_height), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
    ImGui::PopStyleVar();
}

static ImVec2 DrawStatusChip(ImDrawList *dl, ImVec2 pos, const char *label, bool on, const char *text = nullptr)
{
    const char *state = text ? text : (on ? "ON" : "OFF");

    const float pad_x = 8.0f, pad_y = 4.0f, gap = 6.0f, rounding = 6.0f;

    ImVec2 label_size = ImGui::CalcTextSize(label);
    ImVec2 state_size = ImGui::CalcTextSize(state);

    ImVec2 size(pad_x * 2.0f + label_size.x + gap + state_size.x,
                pad_y * 2.0f + label_size.y);
    ImVec2 max(pos.x + size.x, pos.y + size.y);

    dl->AddRectFilled(pos, max, IM_COL32(40, 42, 48, 150), rounding);
    dl->AddRect(pos, max, IM_COL32(255, 255, 255, 40), rounding, 0, 1.0f);
    dl->AddLine(ImVec2(pos.x + rounding, pos.y + 1.0f),
                ImVec2(max.x - rounding, pos.y + 1.0f), IM_COL32(255, 255, 255, 30), 1.0f);

    ImVec2 text_pos(pos.x + pad_x, pos.y + pad_y);
    dl->AddText(text_pos, IM_COL32(210, 210, 215, 255), label);

    ImU32 state_col;
    if (text)    state_col = IM_COL32(130, 180, 255, 255);   // custom text: blue
    else if (on) state_col = IM_COL32(110, 220, 130, 255);   // ON: green
    else         state_col = IM_COL32(150, 150, 155, 255);   // OFF: gray

    dl->AddText(ImVec2(text_pos.x + label_size.x + gap, text_pos.y), state_col, state);

    return size;
}

void VortexGUI::draw_editor_viewport(VortexCamera* camera)
{
    if (!show_scene_viewport) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    is_scene_view_visible = ImGui::Begin("Scene View");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    viewport_width = viewportPanelSize.x;
    viewport_height = viewportPanelSize.y;

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    float absoluteX = windowPos.x + contentMin.x;
    float absoluteY = windowPos.y + contentMin.y;

    ImGui::Image((void*)(intptr_t)scene_texture, ImVec2(viewport_width, viewport_height), ImVec2(0, 1), ImVec2(1, 0));

    ImDrawList *dl = ImGui::GetWindowDrawList();
    ImVec2 cursor(absoluteX + 10.0f, absoluteY + 10.0f);

    struct status_Flag { const char *label; bool value; const char *text = nullptr; };
    status_Flag flags[] = {
        { "Snap", VortexKeyboard::get_key(Key::LEFT_CTRL) },
        { "Wireframe", app->is_wireframe_enabled() },
        { "Space", false, (m_current_guizmo_mode == ImGuizmo::WORLD) ? "World" : "Local" },
    };

    for (const status_Flag &f : flags)
    {
        ImVec2 size = DrawStatusChip(dl, cursor, f.label, f.value, f.text);
        cursor.y += size.y + 4.0f;
    }

    if (GLOBAL::current_engine_state == GLOBAL::EngineState::EDITOR)
    {
        if (viewport_height > 0.0f)
        {
            camera->set_aspect_ratio(viewport_width / viewport_height);
        }

        handle_picking(camera, ImVec2(absoluteX, absoluteY));

        static bool was_using_gizmo = false;
        static glm::vec3 start_pos;
        static glm::quat start_rot;
        static glm::vec3 start_scale;

        ImGuizmo::BeginFrame();

        if (!VortexObjectManager::selected_models.empty())
        {
            VortexModel *anchor = *VortexObjectManager::selected_models.begin();
            if (!anchor)
            {
                VortexObjectManager::selected_models.clear();

                ImGui::End();
                ImGui::PopStyleVar();

                return;
            }

            if (g_active_decal_for_gizmo)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(absoluteX, absoluteY, viewport_width, viewport_height);

                glm::mat4 view = camera->getViewMatrix();
                glm::mat4 projection = camera->getProjectionMatrix();

                glm::mat4 parent_world = anchor->get_model_matrix();

                glm::mat4 parent_rot_mat(
                    glm::vec4(glm::normalize(glm::vec3(parent_world[0])), 0.0f),
                    glm::vec4(glm::normalize(glm::vec3(parent_world[1])), 0.0f),
                    glm::vec4(glm::normalize(glm::vec3(parent_world[2])), 0.0f),
                    glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
                );
                glm::quat parent_rot = glm::quat_cast(parent_rot_mat);

                glm::mat4 local_matrix = glm::translate(glm::mat4(1.0f), g_active_decal_for_gizmo->position) *
                                         glm::mat4_cast(g_active_decal_for_gizmo->orientation) *
                                         glm::scale(glm::mat4(1.0f), g_active_decal_for_gizmo->scale);
                glm::mat4 true_world = parent_world * local_matrix;
                glm::vec3 world_pos = glm::vec3(true_world[3]);
                glm::quat world_rot = parent_rot * g_active_decal_for_gizmo->orientation;

                glm::mat4 gizmo_matrix = glm::translate(glm::mat4(1.0f), world_pos) * glm::mat4_cast(world_rot);

                if (m_current_op == ImGuizmo::SCALE)
                {
                    gizmo_matrix = glm::scale(gizmo_matrix, g_active_decal_for_gizmo->scale);
                }

                ImGuizmo::OPERATION decal_op = m_current_op;

                if (m_current_op == ImGuizmo::TRANSLATE) decal_op = ImGuizmo::TRANSLATE;
                if (m_current_op == ImGuizmo::ROTATE)    decal_op = ImGuizmo::ROTATE_Y;
                if (m_current_op == ImGuizmo::SCALE)     decal_op = (ImGuizmo::OPERATION)(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Z);

                ImGuizmo::MODE actual_mode = ImGuizmo::LOCAL;

                ImGuizmo::Manipulate(
                    glm::value_ptr(view), glm::value_ptr(projection),
                    decal_op, actual_mode, glm::value_ptr(gizmo_matrix)
                );

                if (ImGuizmo::IsUsing())
                {
                    if (m_current_op == ImGuizmo::TRANSLATE)
                    {
                        ImVec2 mousePos = ImGui::GetMousePos();
                        float mouseX = mousePos.x - absoluteX;
                        float mouseY = mousePos.y - absoluteY;
                        float ndcX = (2.0f * mouseX) / viewport_width - 1.0f;
                        float ndcY = 1.0f - (2.0f * mouseY) / viewport_height;

                        glm::mat4 invProj = glm::inverse(projection);
                        glm::mat4 invView = glm::inverse(view);
                        glm::vec4 ray_clip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
                        glm::vec4 ray_eye = invProj * ray_clip;
                        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
                        glm::vec3 mouse_ray_dir = glm::normalize(glm::vec3(invView * ray_eye));

                        RaycastHit hit = VortexPhysics::editor_raycast(camera->get_position(), mouse_ray_dir, 1000.0f);

                        if (hit.has_hit && hit.hit_model == anchor)
                        {
                            glm::vec3 snap_pos = hit.hit_point + (hit.hit_normal * 0.01f);

                            glm::quat current_world_rot = parent_rot * g_active_decal_for_gizmo->orientation;

                            glm::vec3 current_z_axis = glm::normalize(current_world_rot * glm::vec3(0.0f, 0.0f, 1.0f));

                            glm::vec3 new_z_axis = current_z_axis - hit.hit_normal * glm::dot(current_z_axis, hit.hit_normal);

                            if (glm::length(new_z_axis) < 0.001f)
                            {
                                glm::vec3 safe_up = (std::abs(hit.hit_normal.y) > 0.9f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);
                                new_z_axis = safe_up - hit.hit_normal * glm::dot(safe_up, hit.hit_normal);
                            }

                            new_z_axis = glm::normalize(new_z_axis);

                            glm::vec3 new_x_axis = glm::normalize(glm::cross(hit.hit_normal, new_z_axis));

                            glm::mat4 snap_rot_mat(
                                glm::vec4(new_x_axis, 0.0f),
                                glm::vec4(hit.hit_normal, 0.0f),
                                glm::vec4(new_z_axis, 0.0f),
                                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
                            );

                            glm::mat4 inv_parent_full = glm::inverse(parent_world);
                            g_active_decal_for_gizmo->position = glm::vec3(inv_parent_full * glm::vec4(snap_pos, 1.0f));

                            g_active_decal_for_gizmo->orientation = glm::inverse(parent_rot) * glm::quat_cast(snap_rot_mat);
                        }
                    }
                    else if (m_current_op == ImGuizmo::ROTATE)
                    {
                        glm::mat4 rot_only(
                            glm::vec4(glm::normalize(glm::vec3(gizmo_matrix[0])), 0.0f),
                            glm::vec4(glm::normalize(glm::vec3(gizmo_matrix[1])), 0.0f),
                            glm::vec4(glm::normalize(glm::vec3(gizmo_matrix[2])), 0.0f),
                            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
                        );
                        glm::quat new_world_rot = glm::quat_cast(rot_only);
                        g_active_decal_for_gizmo->orientation = glm::inverse(parent_rot) * new_world_rot;
                    }
                    else if (m_current_op == ImGuizmo::SCALE)
                    {
                        float mTrans[3], mRot[3], mScale[3];
                        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(gizmo_matrix), mTrans, mRot, mScale);

                        g_active_decal_for_gizmo->scale.x = mScale[0];
                        g_active_decal_for_gizmo->scale.z = mScale[2];
                    }
                }

                is_using_gizmo = ImGuizmo::IsUsing();

                if (is_using_gizmo && !was_using_gizmo)
                {
                    start_pos   = g_active_decal_for_gizmo->position;
                    start_rot   = g_active_decal_for_gizmo->orientation;
                    start_scale = g_active_decal_for_gizmo->scale;
                }

                if (!is_using_gizmo && was_using_gizmo)
                {
                    if (start_pos != g_active_decal_for_gizmo->position ||
                        start_rot != g_active_decal_for_gizmo->orientation ||
                        start_scale != g_active_decal_for_gizmo->scale)
                    {
                        ActionManager::push_action(new ActionDecalTransform(
                            g_active_decal_for_gizmo,
                            start_pos, g_active_decal_for_gizmo->position,
                            start_rot, g_active_decal_for_gizmo->orientation,
                            start_scale, g_active_decal_for_gizmo->scale
                        ));
                    }
                }

                was_using_gizmo = is_using_gizmo;
                m_is_using_gizmo = is_using_gizmo;
            }
            else
            {
                if (m_current_op == 0) m_current_op = ImGuizmo::TRANSLATE;

                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(absoluteX, absoluteY, viewport_width, viewport_height);

                glm::mat4 view = camera->getViewMatrix();
                glm::mat4 projection = camera->getProjectionMatrix();

                glm::mat4 anchor_old_matrix = anchor->get_model_matrix();
                glm::mat4 gizmo_matrix = anchor_old_matrix;

                float snap_values[3] = { 1.0f, 1.0f, 1.0f };
                if (m_current_op == ImGuizmo::ROTATE) { snap_values[0] = 15.0f; snap_values[1] = 15.0f; snap_values[2] = 15.0f; }

                float* snap_pointer = VortexKeyboard::get_key(Key::LEFT_CTRL) ? snap_values : nullptr;
                ImGuizmo::MODE actual_mode = (m_current_op == ImGuizmo::SCALE) ? ImGuizmo::LOCAL : m_current_guizmo_mode;

                ImGuizmo::Manipulate(
                    glm::value_ptr(view), glm::value_ptr(projection),
                    m_current_op, actual_mode, glm::value_ptr(gizmo_matrix), nullptr, snap_pointer
                );

                is_using_gizmo = ImGuizmo::IsUsing();

                if (is_using_gizmo && !was_using_gizmo)
                {
                    start_pos   = anchor->transform.position;
                    start_rot   = anchor->transform.orientation;
                    start_scale = anchor->transform.scale;
                }

                if (is_using_gizmo)
                {
                    glm::mat4 delta_matrix = gizmo_matrix * glm::inverse(anchor_old_matrix);

                    for (VortexModel* model : VortexObjectManager::selected_models)
                    {
                        if (!model) continue;

                        glm::mat4 final_matrix;
                        if (model == anchor) final_matrix = gizmo_matrix;
                        else final_matrix = delta_matrix * model->get_model_matrix();

                        float mTranslation[3], mRotation[3], mScale[3];
                        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(final_matrix), mTranslation, mRotation, mScale);

                        model->transform.position = glm::vec3(mTranslation[0], mTranslation[1], mTranslation[2]);
                        model->transform.set_euler(glm::vec3(mRotation[0], mRotation[1], mRotation[2]));
                        model->transform.scale = glm::vec3(mScale[0], mScale[1], mScale[2]);
                        model->set_model_matrix(final_matrix);
                    }
                }

                if (!is_using_gizmo && was_using_gizmo)
                {
                    if (start_pos != anchor->transform.position || start_rot != anchor->transform.orientation || start_scale != anchor->transform.scale)
                    {
                        ActionManager::push_action(new ActionTransform(
                            anchor, start_pos, anchor->transform.position,
                            start_rot, anchor->transform.orientation,
                            start_scale, anchor->transform.scale
                        ));
                    }
                }

                was_using_gizmo = is_using_gizmo;
                m_is_using_gizmo = is_using_gizmo;
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void VortexGUI::handle_picking(VortexCamera* camera, ImVec2 image_pos)
{
    if (ImGui::IsWindowHovered() && VortexMouse::get_button_down("LEFT") && !ImGuizmo::IsOver())
    {
        ImVec2 mousePos = ImGui::GetMousePos();

        float mouseX = mousePos.x - image_pos.x;
        float mouseY = mousePos.y - image_pos.y;

        if (mouseX >= 0.0f && mouseX <= viewport_width && mouseY >= 0.0f && mouseY <= viewport_height)
        {
            float ndcX = (2.0f * mouseX) / scene_width - 1.0f;
            float ndcY = 1.0f - (2.0f * mouseY) / scene_height;

            glm::mat4 invProj = glm::inverse(camera->getProjectionMatrix());
            glm::mat4 invView = glm::inverse(camera->getViewMatrix());

            glm::vec4 ray_clip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 ray_eye = invProj * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
            glm::vec3 ray_direction = glm::normalize(glm::vec3(invView * ray_eye));

            RaycastHit hit = VortexPhysics::editor_raycast(camera->get_position(), ray_direction, 1000.0f);

            if (hit.has_hit)
            {
                if (!ImGui::GetIO().KeyCtrl && VortexObjectManager::selected_models.find(hit.hit_model) == VortexObjectManager::selected_models.end()) g_active_decal_for_gizmo = nullptr;

                if (ImGui::GetIO().KeyCtrl)
                {
                    if (VortexObjectManager::selected_models.find(hit.hit_model) != VortexObjectManager::selected_models.end())
                    {
                        VortexObjectManager::selected_models.erase(hit.hit_model);
                        hit.hit_model->is_selected = false;
                    }
                    else
                    {
                        VortexObjectManager::selected_models.insert(hit.hit_model);
                        hit.hit_model->is_selected = true;
                    }
                }
                else
                {
                    for (VortexModel* m : VortexObjectManager::selected_models) m->is_selected = false;
                    VortexObjectManager::selected_models.clear();

                    VortexObjectManager::selected_models.insert(hit.hit_model);
                    hit.hit_model->is_selected = true;
                }
            }
            else
            {
                if (!ImGui::GetIO().KeyCtrl)
                {
                    for (VortexModel* m : VortexObjectManager::selected_models) m->is_selected = false;
                    VortexObjectManager::selected_models.clear();
                }
            }
        }
    }
}
