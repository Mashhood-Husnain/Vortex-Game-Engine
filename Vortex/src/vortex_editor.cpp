#include "vortex_editor.hpp"

void VortexEditor::update(float deltaTime, VortexCamera *camera, VortexApplication *app)
{
    this->app = app;

    handle_shortcuts();
    handle_picking(camera);
    draw_gizmos(camera);
}

void VortexEditor::handle_shortcuts()
{
    if (VortexKeyboard::get_key_down("T")) current_op = ImGuizmo::TRANSLATE;
    if (VortexKeyboard::get_key_down("R")) current_op = ImGuizmo::ROTATE;
    if (VortexKeyboard::get_key_down("Y")) current_op = ImGuizmo::SCALE;
    if (VortexKeyboard::get_key_down("LEFTALT")) snap_to_floor();
}

void VortexEditor::handle_picking(VortexCamera* camera)
{
    if (VortexMouse::get_button_down("LEFT") && !ImGuizmo::IsOver() && !ImGui::GetIO().WantCaptureMouse)
    {
        glm::vec3 ray_direction = camera->get_ray_from_mouse(VortexMouse::get_position(), app);
        RaycastHit hit = VortexPhysics::editor_raycast(camera->position, ray_direction, 1000.0f);

        if (hit.has_hit)
        {
            selected_model = hit.hit_model;
            selected_model->is_selected = true;
        }
        else
        {
            if (selected_model) selected_model->is_selected = false;
            selected_model = nullptr;
        }

    }
}

void VortexEditor::draw_gizmos(VortexCamera* camera)
{
    if (!selected_model) return;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = camera->getProjectionMatrix();

    glm::mat4 model_matrix = selected_model->get_model_matrix();

    float snap_values[3] = { 1.0f, 1.0f, 1.0f };
    if (current_op == ImGuizmo::ROTATE) {
        snap_values[0] = 15.0f;
        snap_values[1] = 15.0f;
        snap_values[2] = 15.0f;
    }

    float* snap_pointer = VortexKeyboard::get_key("LEFTCONTROL") ? snap_values : nullptr;

    ImGuizmo::Manipulate(
        glm::value_ptr(view), 
        glm::value_ptr(projection), 
        current_op, 
        ImGuizmo::LOCAL, 
        glm::value_ptr(model_matrix),
        nullptr,
        snap_pointer
    );

    if (ImGuizmo::IsUsing())
    {
        is_using_gizmo = true;
        
        float mTranslation[3], mRotation[3], mScale[3];
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(model_matrix), 
            mTranslation, 
            mRotation, 
            mScale
        );

        selected_model->transform.position = glm::vec3(mTranslation[0], mTranslation[1], mTranslation[2]);
        selected_model->transform.set_euler(glm::vec3(mRotation[0], mRotation[1], mRotation[2]));
        selected_model->transform.scale = glm::vec3(mScale[0], mScale[1], mScale[2]);

        selected_model->set_model_matrix(model_matrix);
    }
    else
    {
        is_using_gizmo = false;
    }
}

void VortexEditor::snap_to_floor()
{
    if (!selected_model) return;

    auto& objects = selected_model->get_objects();
    if (objects.empty()) return;

    float lowest_local_y = 1e10f;
    for (const auto& obj : objects)
    {
        float obj_bottom = obj.collider.min.y + obj.transform.position.y;
        if (obj_bottom < lowest_local_y) {
            lowest_local_y = obj_bottom;
        }
    }

    float scaled_lowest_y = lowest_local_y * selected_model->transform.scale.y;

    selected_model->transform.set_position(
        glm::vec3(
            selected_model->transform.get_position().x, 
            -scaled_lowest_y,
            selected_model->transform.get_position().z
        )
    );

    selected_model->set_model_matrix(selected_model->get_model_matrix());
}

VortexEditor::~VortexEditor()
{
    app = nullptr;
    selected_model = nullptr;    
}
