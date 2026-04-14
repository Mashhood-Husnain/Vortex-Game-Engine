#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"

class PlayerBulletMover : public VortexMonoBehaviour
{
private:
    float speed = 100.0f;
    float distance_traveled = 0.0f;
    glm::vec3 fly_direction;
    float bullet_damage = 5.0f;
    VortexModel *player;
public:
    void on_start() override
    {
        fly_direction = gameObject->app->camera->front;

        fly_direction = glm::normalize(fly_direction);

        player = VortexObjectManager::get_object_by_tag("Player");
    }

    void on_update(float deltaTime) override
    {
        gameObject->transform.position += fly_direction * speed * deltaTime;
        distance_traveled += speed * deltaTime;

        if (distance_traveled >= 200.0f)
        {
            gameObject->should_destroy = true;
        }

        for (VortexModel* target_model : VortexObjectManager::active_models) 
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
                VortexMonoBehaviour *script = player->behaviours[0];
                VortexMonoBehaviour *enemy_script = target_model->behaviours[0];

                if (enemy_script)
                {
                    enemy_script->VortexMonoBehaviour_set_value(
                        "enemy_health",
                        enemy_script->VortexMonoBehaviour_get_value("enemy_health") - bullet_damage
                    );
                }

                if (script && enemy_script->VortexMonoBehaviour_get_value("enemy_health") <= 0.0f)
                {
                    script->VortexMonoBehaviour_set_value(
                        "player_points", script->VortexMonoBehaviour_get_value("player_points") + 1.0f
                    );
                }

                gameObject->should_destroy = true;

                return;
            }
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletMover);
