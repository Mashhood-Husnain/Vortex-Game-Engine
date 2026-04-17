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

#include <cstdlib>

class PlayerBulletMover : public VortexMonoBehaviour
{
private:
    float speed = 150.0f;
    float distance_traveled = 0.0f;
    glm::vec3 fly_direction;
    float bullet_damage = 5.0f;
    VortexModel *player;
    VortexModel *gun;
public:
    void on_start() override
    {
        player = VortexObjectManager::get_object_by_tag("Player");
        gun = VortexObjectManager::get_object_by_tag("Gun");

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
        float move_dist = speed * deltaTime;

        RaycastHit hit = VortexPhysics::raycast(gameObject->transform.position, fly_direction, move_dist, {gameObject, player, gun});

        if (hit.has_hit)
        {
            gameObject->transform.position = hit.hit_point;

            bool is_destructible = false;
            for (const std::string& script_name : hit.hit_model->script_names)
            {
                if (script_name == "VortexTagDestructible")
                {
                    is_destructible = true;
                    break;
                }
            }

            if (is_destructible && hit.hit_sub_object_index != -1)
            {
                VortexAudio::play_sound("assets/audio/gunshot_hit.wav", 0.5f);
                hit.hit_model->active_parts[hit.hit_sub_object_index] = false;
            }

            bool is_enemy = false;
            VortexMonoBehaviour *enemy_script = nullptr;
            
            for (size_t i = 0; i < hit.hit_model->script_names.size(); i++) 
            {
                if (hit.hit_model->script_names[i] == "EnemyMovement") {
                    is_enemy = true;
                    enemy_script = hit.hit_model->behaviours[i];
                    break;
                }
            }

            if (is_enemy && enemy_script)
            {
                VortexAudio::play_sound("assets/audio/gunshot_hit.wav", 0.5f);
                float current_hp = enemy_script->VortexMonoBehaviour_get_value("enemy_health");
                enemy_script->VortexMonoBehaviour_set_value("enemy_health", current_hp - bullet_damage);

                if (player && !player->behaviours.empty()) 
                {
                    VortexMonoBehaviour *player_script = player->behaviours[0];
                    if ((current_hp - bullet_damage) <= 0.0f)
                    {
                        float pts = player_script->VortexMonoBehaviour_get_value("player_points");
                        player_script->VortexMonoBehaviour_set_value("player_points", pts + 1.0f);
                    }
                }
            }

            gameObject->should_destroy = true;
            return;
        }

        gameObject->transform.position += fly_direction * move_dist;
        distance_traveled += move_dist;

        if (distance_traveled >= 200.0f)
        {
            gameObject->should_destroy = true;
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletMover);
