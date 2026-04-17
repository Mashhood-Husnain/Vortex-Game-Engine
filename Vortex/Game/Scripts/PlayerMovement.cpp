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
    bool is_grounded = false;
    float movement_speed = 15.0f;

    glm::vec2 crosshair;

    VortexModel *gun;
    VortexModel *bullet;
    VortexModel *floor;
public:

    void on_start() override
    {
        crosshair = VortexHUD::GetScreenDimensions();
        gun = VortexObjectManager::get_object_by_tag("Gun");
        bullet = VortexObjectManager::get_object_by_tag("Bullet");
        floor = VortexObjectManager::get_object_by_tag("Floor");
    }

    void on_update(float deltaTime) override
    {
        draw_crosshair();

        glm::vec3 cam_forward = gameObject->app->camera->front;
        glm::vec3 cam_right = gameObject->app->camera->right;
        cam_forward.y = 0.0f;
        cam_right.y = 0.0f;
        cam_forward = glm::normalize(cam_forward);
        cam_right = glm::normalize(cam_right);

        glm::vec3 move_dir(0.0f);
        if (VortexKeyboard::get_key("W")) move_dir += cam_forward;
        if (VortexKeyboard::get_key("S")) move_dir -= cam_forward;
        if (VortexKeyboard::get_key("A")) move_dir -= cam_right;
        if (VortexKeyboard::get_key("D")) move_dir += cam_right;

        if (glm::length(move_dir) > 0.0f) {
            move_dir = glm::normalize(move_dir) * movement_speed * deltaTime;
        }

        move_and_collide(glm::vec3(move_dir.x, 0.0f, 0.0f));
        move_and_collide(glm::vec3(0.0f, 0.0f, move_dir.z));

        if (is_grounded && VortexKeyboard::get_key("SPACE"))
        {
            velocity_y = jump_force;
            is_grounded = false;
        }

        velocity_y -= gravity * deltaTime;
        gameObject->transform.position.y += velocity_y * deltaTime;
        
        is_grounded = false;
        
        for (VortexModel* other : VortexObjectManager::active_models)
        {
            if (other == gameObject || !other->is_active) continue;
            if (other == bullet || other == gun) continue;
            if (is_enemy(other)) continue;
            
            if (VortexPhysics::check_collision(gameObject, other))
            {
                gameObject->transform.position.y -= velocity_y * deltaTime;

                if (velocity_y < 0.0f) is_grounded = true; 
                velocity_y = 0.0f;
                break;
            }
        }
    }

    void move_and_collide(glm::vec3 move_amount)
    {
        gameObject->transform.position += move_amount;

        for (VortexModel* other : VortexObjectManager::active_models)
        {
            if (other == gameObject || !other->is_active) continue;
            if (other == bullet || other == gun || other == floor) continue;

            if (is_enemy(other)) continue;

            CollisionHit hit = VortexPhysics::check_collision_detailed(gameObject, other);
            
            if (hit.has_hit)
            {
                gameObject->transform.position -= move_amount;
                break;
            }
        }
    }

    bool is_enemy(VortexModel *model)
    {
        bool is_enemy = false;
        for (const std::string& s_name : model->script_names)
        {
            if (s_name == "EnemyMovement")
            {
                is_enemy = true;
                break;
            }
        }

        return is_enemy;
    }

    void draw_crosshair()
    {
        VortexHUD::Begin();
        VortexHUD::Rect(ImVec2(crosshair.x / 2, crosshair.y / 2), ImVec2(10, 10), ImVec4(0, 1, 0, 1), 2.0f);
        VortexHUD::End();
    }
};

VORTEX_REGISTER_SCRIPT(PlayerMovement);
