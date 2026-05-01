#include "VortexEngine.hpp"

class PlayerMovement : public VortexMonoBehaviour
{
private:
    float jump_force = 30.0f;
    float velocity_y = 0.0f;
    float gravity = 25.0f;
    bool is_grounded = false;
    float movement_speed = 2.0f;

    glm::vec2 crosshair;

    VortexModel *gun;
    VortexModel *bullet;
    VortexModel *floor;
public:

    PlayerMovement()
    {
        expose_float("Gravity", &gravity, true);
    }

    void on_start() override
    {
        crosshair = VortexHUD::GetScreenDimensions();
        gun = VortexObjectManager::get_object_by_tag("Gun");
        bullet = VortexObjectManager::get_object_by_tag("Bullet");
        floor = VortexObjectManager::get_object_by_tag("Floor");
    }

    void on_update(float deltaTime) override
    {
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        Vec3 cam_forward = engine().get_camera()->get_front();
        Vec3 cam_right = engine().get_camera()->get_right();
        cam_forward.y = 0.0f;
        cam_right.y = 0.0f;
        cam_forward = glm::normalize(cam_forward);
        cam_right = glm::normalize(cam_right);

        Vec3 move_dir(0.0f);
        if (VortexKeyboard::get_key("W")) move_dir += cam_forward;
        if (VortexKeyboard::get_key("S")) move_dir -= cam_forward;
        if (VortexKeyboard::get_key("A")) move_dir -= cam_right;
        if (VortexKeyboard::get_key("D")) move_dir += cam_right;

        if (glm::length(move_dir) > 0.0f)
        {
            move_dir = glm::normalize(move_dir) * movement_speed * deltaTime;
        }

        move_and_collide(Vec3(move_dir.x, 0.0f, 0.0f));
        move_and_collide(Vec3(0.0f, 0.0f, move_dir.z));

        if (is_grounded && VortexKeyboard::get_key("SPACE"))
        {
            velocity_y = jump_force;
            is_grounded = false;
        }

        velocity_y -= gravity * deltaTime;
        transform().position.y += velocity_y * deltaTime;
        
        is_grounded = false;
        
        for (VortexModel* other : VortexObjectManager::active_models)
        {
            if (other == &object() || !other->is_active) continue;
            if (other == bullet || other == gun) continue;
            if (other->get_behaviour("EnemyMovement")) continue;
            
            if (VortexPhysics::check_collision(&object(), other))
            {
                transform().position.y -= velocity_y * deltaTime;

                if (velocity_y < 0.0f) is_grounded = true; 
                velocity_y = 0.0f;
                break;
            }
        }
    }

    void move_and_collide(Vec3 move_amount)
    {
        transform().position += move_amount;

        for (VortexModel* other : VortexObjectManager::active_models)
        {
            if (other == &object() || !other->is_active) continue;
            if (other == bullet || other == gun || other == floor) continue;

            if (other->get_behaviour("EnemyMovement")) continue;

            CollisionHit hit = VortexPhysics::check_collision_detailed(&object(), other);
            
            if (hit.has_hit)
            {
                transform().position -= move_amount;
                break;
            }
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerMovement);
