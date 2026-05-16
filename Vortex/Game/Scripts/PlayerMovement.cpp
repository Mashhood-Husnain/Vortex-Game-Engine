#include "VortexEngine.hpp"

class PlayerMovement : public VortexMonoBehaviour
{
private:
    const float movement_speed = 17.0f;
    const float jump_force = 10.0f;
    float velocity_y = 0.0f;
    bool is_grounded = true;
    const float gravity = 20.0f;
public:
    void on_start() override
    {
        // Initialization
    }

    void on_update(float deltaTime) override
    {
        Vec3 cam_forward = engine().get_camera()->get_front();
        Vec3 cam_right = engine().get_camera()->get_right();

        cam_forward.y = cam_right.y = 0.0f;

        cam_forward = normalize(cam_forward);
        cam_right = normalize(cam_right);

        Vec3 move_dir(0.0f);
        if(VortexKeyboard::get_key("W")) move_dir += cam_forward;
        if(VortexKeyboard::get_key("S")) move_dir -= cam_forward;
        if(VortexKeyboard::get_key("A")) move_dir -= cam_right;
        if(VortexKeyboard::get_key("D")) move_dir += cam_right;

        if (length(move_dir) > 0.0f) move_dir = normalize(move_dir) * movement_speed * deltaTime;

        transform().position += move_dir;

        if (is_grounded && VortexKeyboard::get_key("SPACE"))
        {
            velocity_y = jump_force;
            is_grounded = false;
        }

        // record previous frame y position
        float previous_y = transform().position.y;

        velocity_y -= gravity * deltaTime * !is_grounded;
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
