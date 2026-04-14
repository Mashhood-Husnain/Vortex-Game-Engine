#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_audio.hpp"

class PlayerBulletMover : public VortexMonoBehaviour
{
private:
    float speed = 150.0f;
    float distance_traveled = 0.0f;
    glm::vec3 fly_direction;
    float bullet_damage = 5.0f;
    VortexModel *player;
public:
    void on_start() override
    {
        player = VortexObjectManager::get_object_by_tag("Player");
        VortexCamera* cam = gameObject->app->camera;

        glm::vec3 crosshair_target = cam->position + (cam->front * 100.0f);

        glm::vec3 base_direction = glm::normalize(crosshair_target - gameObject->transform.position);

        bool is_aiming = VortexMouse::get_button("RIGHT");
        float spread_factor = is_aiming ? 0.005f : 0.05f; 

        glm::vec3 spread(
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor,
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor,
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor
        );

        fly_direction = glm::normalize(base_direction + spread);
    }

    void on_update(float deltaTime) override
    {
        gameObject->transform.position += fly_direction * speed * deltaTime;
        distance_traveled += speed * deltaTime;

        if (distance_traveled >= 200.0f)
        {
            gameObject->should_destroy = true;
            return;
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
                VortexAudio::play_sound("assets/audio/gunshot_hit.wav", 0.5f);

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
