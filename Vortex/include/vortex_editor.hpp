#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include "vortex_model.hpp"
#include "vortex_camera.hpp"
#include "vortex_mouse.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_physics.hpp"
#include "vortex_application.hpp"
#include "util/vortex_logs.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <ImGuizmo.hpp>
#include <imgui.h>

class VortexEditor
{
    VortexApplication *app = nullptr;
public:
    VortexModel *selected_model = nullptr;

    ImGuizmo::OPERATION current_op = ImGuizmo::TRANSLATE;
    bool is_using_gizmo = false;

    void update(float deltaTime, VortexCamera *camera, VortexApplication *app);

    void handle_picking(VortexCamera *camera);
    void draw_gizmos(VortexCamera *camera);
    void handle_shortcuts();

    void snap_to_floor();

    ~VortexEditor();
};
