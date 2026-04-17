#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_audio.hpp"

class EnemyMovement : public VortexMonoBehaviour
{
private:
    float speed = 5.0f;
    float health = 20.0f;
    float enemy_damage = 5.5f;
    VortexModel *target_player = nullptr;
public:
    void on_start() override
    {
        VortexMonoBehaviour_set_value("enemy_health", health);
        target_player = VortexObjectManager::get_object_by_tag("Player");
    }

    void on_update(float deltaTime) override
    {
        if (!target_player || target_player->should_destroy) return; 
        if (gameObject->transform.position.y >= 0.25f)
        {
            gameObject->should_destroy = true;
            return;
        }

        health = VortexMonoBehaviour_get_value("enemy_health");

        if (health <= 0.0f)
        {
            gameObject->should_destroy = true;
            return;
        }

        glm::vec3 player_pos = target_player->transform.position;
        player_pos.y = 0.0f;

        glm::vec3 my_pos = gameObject->transform.position;
        my_pos.y = 0.0f;

        glm::vec3 direction = player_pos - my_pos;

        if (glm::length(direction) > 0.1f) 
        {
            direction = glm::normalize(direction);
            gameObject->transform.position += direction * speed * deltaTime;

            float angle = atan2(direction.x, direction.z);
            gameObject->transform.rotation.y = glm::degrees(angle);
        }

        if (VortexPhysics::check_collision(gameObject, target_player))
        {
            VortexMonoBehaviour *script = target_player->behaviours[0];

            if (script)
            {
                float current_cooldown = script->VortexMonoBehaviour_get_value("player_damage_cooldown");

                if (current_cooldown >= 1.0f)
                {
                    script->VortexMonoBehaviour_set_value("player_damage_cooldown", 0.0f);

                    float current_hp = script->VortexMonoBehaviour_get_value("player_health");
                    script->VortexMonoBehaviour_set_value("player_health", current_hp - enemy_damage);

                    VortexAudio::play_sound("assets/audio/player_damage.wav", 1.0f);
                }
            }
        }

        draw_health_bar();
    }

    void draw_health_bar()
    {
        VortexHUD::Begin();

        glm::vec3 label_pos = gameObject->transform.position + glm::vec3(0, 2.5f, 0);
        VortexHUD::WorldLabel("HEALTH", label_pos, gameObject->app->camera, ImVec4(1, 1, 1, 1));

        glm::vec3 bar_pos = gameObject->transform.position + glm::vec3(0, 2.2f, 0);
        VortexHUD::WorldBar(health, 20.0f, bar_pos, gameObject->app->camera, ImVec2(80, 8), ImVec4(1, 0, 0, 1));

        VortexHUD::End();
    }
};

VORTEX_REGISTER_SCRIPT(EnemyMovement);
