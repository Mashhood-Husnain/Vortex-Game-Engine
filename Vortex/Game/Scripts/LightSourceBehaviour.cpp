#include "VortexEngine.hpp"

class LightSourceBehaviour : public VortexMonoBehaviour
{
private:
    float movement_speed = 4.0f;
    float radius = 10.0f;
    float angle = 0.0f;

    Vec3 center_point;
public:

    LightSourceBehaviour()
    {
        expose_float("Speed", &movement_speed, true);
        expose_float("Radius", &radius, true);
    }

    void on_start() override
    {
        center_point = transform().position;
    }

    void on_update(float deltaTime) override
    {
        angle += movement_speed * deltaTime;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        transform().position = Vec3(
            center_point.x + x,
            center_point.y,
            center_point.z + z
        );
    }
};

VORTEX_REGISTER_SCRIPT(LightSourceBehaviour);
