#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_audio.hpp"
#include "vortex_rigidbody.hpp"

class PlayerMovement : public VortexMonoBehaviour
{
private:
    float jump_force = 15.0f;
    float velocity_y = 0.0f;
    float gravity = 25.0f;
    bool is_grounded = true;
    float movement_speed = 15.0f;

    glm::vec2 crosshair;
public:

    void on_start() override
    {
        crosshair = VortexHUD::GetScreenDimensions();
    }

    void on_update(float deltaTime) override
    {
        draw_crosshair();

        if (is_grounded && VortexKeyboard::get_key_down("SPACE"))
        {
            is_grounded = false;
            velocity_y = jump_force;
        }

        if (!is_grounded)
        {
            velocity_y -= gravity * deltaTime;
            gameObject->transform.position.y += velocity_y * deltaTime;

            if (gameObject->transform.position.y <= 0.0f)
            {
                gameObject->transform.position.y = 0.0f;
                velocity_y = 0.0f;
                is_grounded = true;
            }
        }

        glm::vec3 cam_forward = gameObject->app->camera->front;
        glm::vec3 cam_right = gameObject->app->camera->right;

        cam_forward.y = 0.0f;
        cam_right.y = 0.0f;

        cam_forward = glm::normalize(cam_forward);
        cam_right = glm::normalize(cam_right);

        if (VortexKeyboard::get_key("W")) gameObject->transform.position += cam_forward * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("S")) gameObject->transform.position -= cam_forward * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("A")) gameObject->transform.position -= cam_right * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("D")) gameObject->transform.position += cam_right * movement_speed * deltaTime;
    }

    void draw_crosshair()
    {
        VortexHUD::Begin();

        VortexHUD::Rect(ImVec2(crosshair.x / 2, crosshair.y / 2), ImVec2(10, 10), ImVec4(0, 1, 0, 1), 2.0f);

        VortexHUD::End();
    }
};

VORTEX_REGISTER_SCRIPT(PlayerMovement);
