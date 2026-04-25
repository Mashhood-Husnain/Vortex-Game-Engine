#include "vortex_gui.hpp"
#include "vortex_application.hpp"
#include "vortex_physics.hpp"

void VortexGUI::draw_editor_viewport(float deltaTime, VortexCamera* camera)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("Scene View");

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    viewport_width = static_cast<int>(viewportPanelSize.x);
    viewport_height = static_cast<int>(viewportPanelSize.y);

    ImVec2 image_render_pos = ImGui::GetCursorScreenPos();

    ImGui::Image((void*)(intptr_t)scene_texture, ImVec2(scene_width, scene_height), ImVec2(0, 1), ImVec2(1, 0));

    if (app->current_state == EngineState::EDITOR)
    {
        handle_shortcuts();
        handle_picking(camera, image_render_pos);

        ImGuizmo::BeginFrame();

        if (m_selected_model)
        {
            if (m_current_op == 0) m_current_op = ImGuizmo::TRANSLATE;

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
            ImVec2 contentMax = ImGui::GetWindowContentRegionMax();

            float viewportX = windowPos.x + contentMin.x;
            float viewportY = windowPos.y + contentMin.y;
            float viewportWidth = contentMax.x - contentMin.x;
            float viewportHeight = contentMax.y - contentMin.y;

            ImGuizmo::SetRect(image_render_pos.x, image_render_pos.y, scene_width, scene_height);

            glm::mat4 view = camera->getViewMatrix();
            glm::mat4 projection = camera->getProjectionMatrix();
            glm::mat4 model_matrix = m_selected_model->get_model_matrix();

            float snap_values[3] = { 1.0f, 1.0f, 1.0f };
            if (m_current_op == ImGuizmo::ROTATE)
            {
                snap_values[0] = 15.0f; snap_values[1] = 15.0f; snap_values[2] = 15.0f;
            }

            float* snap_pointer = VortexKeyboard::get_key("LEFTCONTROL") ? snap_values : nullptr;

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), m_current_op, ImGuizmo::LOCAL, glm::value_ptr(model_matrix), nullptr, snap_pointer);

            if (ImGuizmo::IsUsing())
            {
                m_is_using_gizmo = true;
                float mTranslation[3], mRotation[3], mScale[3];
                ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model_matrix), mTranslation, mRotation, mScale);

                m_selected_model->transform.position = glm::vec3(mTranslation[0], mTranslation[1], mTranslation[2]);
                m_selected_model->transform.set_euler(glm::vec3(mRotation[0], mRotation[1], mRotation[2]));
                m_selected_model->transform.scale = glm::vec3(mScale[0], mScale[1], mScale[2]);
                m_selected_model->set_model_matrix(model_matrix);
            }
            else
            {
                m_is_using_gizmo = false;
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void VortexGUI::handle_shortcuts()
{
    if (ImGui::IsWindowFocused())
    {
        if (VortexKeyboard::get_key_down("T")) m_current_op = ImGuizmo::TRANSLATE;
        if (VortexKeyboard::get_key_down("R")) m_current_op = ImGuizmo::ROTATE;
        if (VortexKeyboard::get_key_down("Y")) m_current_op = ImGuizmo::SCALE;
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

        if (mouseX >= 0.0f && mouseX <= scene_width && mouseY >= 0.0f && mouseY <= scene_height)
        {
            float ndcX = (2.0f * mouseX) / scene_width - 1.0f;
            float ndcY = 1.0f - (2.0f * mouseY) / scene_height; 

            glm::mat4 invProj = glm::inverse(camera->getProjectionMatrix());
            glm::mat4 invView = glm::inverse(camera->getViewMatrix());

            glm::vec4 ray_clip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
            glm::vec4 ray_eye = invProj * ray_clip;
            ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
            glm::vec3 ray_direction = glm::normalize(glm::vec3(invView * ray_eye));

            RaycastHit hit = VortexPhysics::editor_raycast(camera->position, ray_direction, 1000.0f);

            if (hit.has_hit)
            {
                m_selected_model = hit.hit_model;
                m_selected_model->is_selected = true;
                VORTEX_INFO("[EDITOR] Selected: ", m_selected_model->model_name);
            }
            else
            {
                if (m_selected_model) m_selected_model->is_selected = false;
                m_selected_model = nullptr;
            }
        }
    }
}

void VortexGUI::snap_to_floor()
{
    if (!m_selected_model) return;

    auto& objects = m_selected_model->get_objects();
    if (objects.empty()) return;

    float lowest_local_y = 1e10f;
    for (const auto& obj : objects)
    {
        float obj_bottom = obj.collider.min.y + obj.transform.position.y;
        if (obj_bottom < lowest_local_y) lowest_local_y = obj_bottom;
    }

    float scaled_lowest_y = lowest_local_y * m_selected_model->transform.scale.y;

    m_selected_model->transform.set_position(
        glm::vec3(m_selected_model->transform.get_position().x, -scaled_lowest_y, m_selected_model->transform.get_position().z)
    );

    m_selected_model->set_model_matrix(m_selected_model->get_model_matrix());
}
