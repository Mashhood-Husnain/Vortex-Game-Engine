#include "VortexEngine.hpp"

class DroneLaserBehaviour : public VortexMonoBehaviour
{
private:
    float life_timer = 1.5f;
    const float damage = 50.0f;
    const float rotation_speed = 5.0f;
public:
    void on_start() override
    {
        // Initialization
    }

    void on_update(float deltaTime) override
    {
        transform().rotate(Vec3(0.0f, 1.0f, 0.0f), rotation_speed * deltaTime, Space::LOCAL);

        life_timer -= deltaTime;
        if (life_timer < 0.0f)
        {
            VortexObjectManager::destroy_object(&object());
        }
    }

    void late_update(float deltaTime) override
    {
        // End of frame
    }

    void on_message(const std::string &message, void *data) override
    {
        // Cross-script communication
    }
};

VORTEX_REGISTER_SCRIPT(DroneLaserBehaviour);
