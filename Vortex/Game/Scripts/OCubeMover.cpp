#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"
#include "util/vortex_keyboard.hpp"
#include "util/vortex_mouse.hpp"

class OCubeMover : public VortexMonoBehaviour
{
private:
    float speed = 100.0f;
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
        gameObject->position += fly_direction * speed * deltaTime;
        distance_traveled += speed * deltaTime;

        if (distance_traveled >= 200.0f)
        {
            gameObject->should_destroy = true;
        }
    }
};

VORTEX_REGISTER_SCRIPT(OCubeMover);
