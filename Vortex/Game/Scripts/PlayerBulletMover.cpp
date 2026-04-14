#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"

#include "vortex_model.hpp"

class PlayerBulletMover : public VortexMonoBehaviour
{
private:
    float speed = 50.0f;
    float distance_traveled = 0.0f;
    glm::vec3 fly_direction;
public:
    void on_start() override
    {
        fly_direction = gameObject->app->camera->front;
        fly_direction.y = 0.0f;

        fly_direction = glm::normalize(fly_direction);
    }

    void on_update(float deltaTime) override
    {
        gameObject->transform.position += fly_direction * speed * deltaTime;
        distance_traveled += speed * deltaTime;

        if (distance_traveled >= 200.0f)
        {
            gameObject->should_destroy = true;
        }

        for (VortexModel* target_model : gameObject->app->dynamic_models) 
        {
            if (target_model == gameObject) continue;
            if (!target_model->is_active || target_model->should_destroy) continue;

            bool is_enemy = false;
            for (const std::string& script_name : target_model->script_names) 
            {
                if (script_name == "EnemyMovement") {
                    is_enemy = true;
                    break;
                }
            }

            if (is_enemy && VortexPhysics::check_collision(gameObject, target_model)) 
            {
                std::cout << "[COMBAT] Enemy Destroyed!" << std::endl;
                    
                target_model->should_destroy = true; 
                
                gameObject->should_destroy = true; 
                
                return; 
            }
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletMover);
