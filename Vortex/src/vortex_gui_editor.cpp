#include "vortex_gui.hpp"
#include "vortex_application.hpp"
#include "vortex_physics.hpp"

void VortexGUI::draw_editor_render_viewport(float deltaTime, VortexCamera *camera)
{
    if (!show_render_scene_viewport) return;

    // TODO: automatically take up all available space (when scene view is not active)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (app->get_state() == EngineState::PLAY)
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

void VortexGUI::draw_editor_viewport(float deltaTime, VortexCamera* camera)
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

    if (app->get_state() == EngineState::EDITOR)
    {
        handle_shortcuts();

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

        if (!m_selected_models.empty())
        {
            VortexModel *anchor = *m_selected_models.begin();

            if (!anchor)
            {
                m_selected_models.clear();
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            if (m_current_op == 0) m_current_op = ImGuizmo::TRANSLATE;

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImGuizmo::SetRect(absoluteX, absoluteY, viewport_width, viewport_height);

            glm::mat4 view = camera->getViewMatrix();
            glm::mat4 projection = camera->getProjectionMatrix();

            glm::mat4 anchor_old_matrix = anchor->get_model_matrix();
            glm::mat4 gizmo_matrix = anchor_old_matrix;

            float snap_values[3] = { 1.0f, 1.0f, 1.0f };
            if (m_current_op == ImGuizmo::ROTATE)
            {
                snap_values[0] = 15.0f; snap_values[1] = 15.0f; snap_values[2] = 15.0f;
            }

            float* snap_pointer = VortexKeyboard::get_key("LEFTCONTROL") ? snap_values : nullptr;

            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(projection),
                m_current_op,
                ImGuizmo::LOCAL,
                glm::value_ptr(gizmo_matrix),
                nullptr,
                snap_pointer
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

                for (VortexModel* model : m_selected_models)
                {
                    if (!model) continue;

                    glm::mat4 final_matrix;

                    if (model == anchor)
                    {
                        final_matrix = gizmo_matrix;
                    }
                    else
                    {
                        final_matrix = delta_matrix * model->get_model_matrix();
                    }

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
                if (start_pos != anchor->transform.position ||
                    start_rot != anchor->transform.orientation ||
                    start_scale != anchor->transform.scale)
                {
                    ActionManager::push_action(new ActionTransform(
                        anchor,
                        start_pos,   anchor->transform.position,
                        start_rot,   anchor->transform.orientation,
                        start_scale, anchor->transform.scale
                    ));
                }
            }

            was_using_gizmo = is_using_gizmo;
            m_is_using_gizmo = is_using_gizmo;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void VortexGUI::handle_shortcuts()
{
    if (ImGui::IsWindowFocused())
    {
        if (VortexKeyboard::get_key_down("T") && !VortexKeyboard::get_key("LEFTCONTROL")) m_current_op = ImGuizmo::TRANSLATE;
        if (VortexKeyboard::get_key_down("R") && !VortexKeyboard::get_key("LEFTCONTROL")) m_current_op = ImGuizmo::ROTATE;
        if (VortexKeyboard::get_key_down("Y") && !VortexKeyboard::get_key("LEFTCONTROL")) m_current_op = ImGuizmo::SCALE;
        if (VortexKeyboard::get_key_down("LEFTALT")) snap_to_floor();
    }
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
                if (ImGui::GetIO().KeyCtrl)
                {
                    if (m_selected_models.find(hit.hit_model) != m_selected_models.end())
                    {
                        m_selected_models.erase(hit.hit_model);
                        hit.hit_model->is_selected = false;
                    }
                    else
                    {
                        m_selected_models.insert(hit.hit_model);
                        hit.hit_model->is_selected = true;
                    }
                }
                else
                {
                    for (VortexModel* m : m_selected_models) m->is_selected = false;
                    m_selected_models.clear();

                    m_selected_models.insert(hit.hit_model);
                    hit.hit_model->is_selected = true;
                }
            }
            else
            {
                if (!ImGui::GetIO().KeyCtrl)
                {
                    for (VortexModel* m : m_selected_models) m->is_selected = false;
                    m_selected_models.clear();
                }
            }
        }
    }
}

void VortexGUI::snap_to_floor()
{
    for (VortexModel* model : m_selected_models)
    {
        auto& objects = model->get_objects();
        if (objects.empty()) continue;

        float lowest_local_y = 1e10f;
        for (const auto& obj : objects)
        {
            float obj_bottom = obj.collider.min.y + obj.transform.position.y;
            if (obj_bottom < lowest_local_y) lowest_local_y = obj_bottom;
        }

        float scaled_lowest_y = lowest_local_y * model->transform.scale.y;

        model->transform.set_position(
            glm::vec3(model->transform.get_position().x, -scaled_lowest_y, model->transform.get_position().z)
        );

        model->set_model_matrix(model->get_model_matrix());
    }
}
