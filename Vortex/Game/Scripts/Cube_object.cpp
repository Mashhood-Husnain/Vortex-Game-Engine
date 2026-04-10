#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"

class Cube_object : public VortexMonoBehaviour
{
public:
    float rotation_speed = 45.0f;
    float jump_force = 10.0f;
    float gravity = 25.0f;
    float velocity_y = 0.0f;

    bool is_grounded = true;

    void on_update(float deltaTime) override
    {
        gameObject->rotation.y += rotation_speed * deltaTime;
        if (gameObject->rotation.y > 360.0f)
        {
            gameObject->rotation.y -= 360.0f;
        }

        if (VortexKeyboard::check_key_pressed("SPACE") && is_grounded)
        {
            velocity_y = jump_force;
            is_grounded = false;
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
    }
};

VORTEX_REGISTER_SCRIPT(Cube_object);
