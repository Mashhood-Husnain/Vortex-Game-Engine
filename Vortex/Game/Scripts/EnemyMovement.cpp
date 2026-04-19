#include "VortexEngine.hpp"

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

        if (health <= 0.0f)
        {
            gameObject->should_destroy = true;
            return;
        }

        Vec3 player_pos = target_player->transform.position;
        player_pos.y = 0.0f;

        Vec3 my_pos = gameObject->transform.position;
        my_pos.y = 0.0f;

        Vec3 direction = player_pos - my_pos;

        if (glm::length(direction) > 0.1f) 
        {
            direction = glm::normalize(direction);
            gameObject->transform.position += direction * speed * deltaTime;

            float angle = atan2(direction.x, direction.z);
            gameObject->transform.orientation.y = glm::degrees(angle);
        }

        if (VortexPhysics::check_collision(gameObject, target_player))
        {
            target_player->send_message("TAKE_DAMAGE", &enemy_damage);
        }

        draw_health_bar();
    }

    void on_message(const std::string &message, void *data) override
    {
        if (message == "TAKE_DAMAGE" && data != nullptr)
        {
            float damage = *static_cast<float*>(data);
            health -= damage;

            if (health <= 0.0f)
            {
                gameObject->should_destroy = true;
                if (target_player)
                {
                    float points_to_add = 1.0f;
                    target_player->send_message("ADD_SCORE", &points_to_add);
                }
            }
        }
    }

    void draw_health_bar()
    {
        VortexHUD::Begin();

        Vec3 label_pos = gameObject->transform.position + Vec3(0, 2.5f, 0);
        VortexHUD::WorldLabel("HEALTH", label_pos, gameObject->app->camera, ImVec4(1, 1, 1, 1));

        Vec3 bar_pos = gameObject->transform.position + Vec3(0, 2.2f, 0);
        VortexHUD::WorldBar(health, 20.0f, bar_pos, gameObject->app->camera, ImVec2(80, 8), ImVec4(1, 0, 0, 1));

        VortexHUD::End();
    }
};

VORTEX_REGISTER_SCRIPT(EnemyMovement);
