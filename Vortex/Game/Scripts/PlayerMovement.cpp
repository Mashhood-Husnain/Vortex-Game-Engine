#include "VortexEngine.hpp"

class PlayerMovement : public VortexMonoBehaviour
{
private:
    const float movement_speed = 10.0f;
    const float jump_force = 10.0f;
    float velocity_y = 0.0f;
    bool is_grounded = true;
public:
    void on_start() override
    {
        // Initialization
    }

    void on_update(float deltaTime) override
    {
        if(VortexKeyboard::get_key("W")) transform().position.x += movement_speed * deltaTime;
        if(VortexKeyboard::get_key("S")) transform().position.x -= movement_speed * deltaTime;
        if(VortexKeyboard::get_key("A")) transform().position.z -= movement_speed * deltaTime;
        if(VortexKeyboard::get_key("D")) transform().position.z += movement_speed * deltaTime;

        if (is_grounded && VortexKeyboard::get_key("SPACE"))
        {
            velocity_y = jump_force;
            is_grounded = false;
        }

        // record previous frame y position
        float previous_y = transform().position.y;

        velocity_y -= GLOBAL::GRAVITY * deltaTime * !is_grounded;
        transform().position.y += velocity_y * deltaTime;

        // check player collision so as to come to a stop
        for (VortexModel *other : VortexObjectManager::active_models)
        {
            if (other == &object() || !other->is_active) continue;
            
            if (VortexPhysics::check_collision_detailed(&object(), other).has_hit)
            {
                if (velocity_y < 0.0f)
                {
                    transform().position.y = previous_y;

                    velocity_y = 0.0f;
                    is_grounded = true;

                    break;
                }
            }
            else
            {
                is_grounded = false;
            }
        }
    }

    void late_update(float deltaTime) override
    {
        // End of frame
    }

    void on_message(const std::string &message, void *data) override
    {
        // Cross-script communication
    }
};

VORTEX_REGISTER_SCRIPT(PlayerMovement);
