#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"

class EnemyMovement : public VortexMonoBehaviour
{
private:
    float speed = 5.0f;
    VortexModel *target_player = nullptr;
public:
    void on_start() override
    {
        for (VortexModel* model : gameObject->app->dynamic_models)
        {
            for (const std::string& script_name : model->script_names)
            {
                if (script_name == "PlayerMovement")
                {
                    target_player = model;
                    return;
                }
            }
        }
    }

    void on_update(float deltaTime) override
    {
        if (!target_player || target_player->should_destroy) return; 

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
    }
};

VORTEX_REGISTER_SCRIPT(EnemyMovement);
