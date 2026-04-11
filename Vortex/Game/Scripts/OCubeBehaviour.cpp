#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"
#include "util/vortex_keyboard.hpp"
#include "util/vortex_mouse.hpp"

class OCubeBehaviour : public VortexMonoBehaviour
{
private:
    // float rotation_speed = 45.0f;
    float jump_force = 20.0f;
    float velocity_y = 0.0f;
    float gravity = 25.0f;
    bool is_grounded = true;
    float movement_speed = 10.0f;
public:

    void on_update(float deltaTime) override
    {
        if (is_grounded && VortexKeyboard::get_key_down("SPACE"))
        {
            is_grounded = false;
            velocity_y = jump_force;
        }

        if (!is_grounded)
        {
            velocity_y -= gravity * deltaTime;
            gameObject->position.y += velocity_y * deltaTime;

            if (gameObject->position.y <= 0.0f)
            {
                gameObject->position.y = 0.0f;
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

        if (VortexKeyboard::get_key("W")) gameObject->position += cam_forward * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("S")) gameObject->position -= cam_forward * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("A")) gameObject->position -= cam_right * movement_speed * deltaTime;
        if (VortexKeyboard::get_key("D")) gameObject->position += cam_right * movement_speed * deltaTime;
    }
};

VORTEX_REGISTER_SCRIPT(OCubeBehaviour);
