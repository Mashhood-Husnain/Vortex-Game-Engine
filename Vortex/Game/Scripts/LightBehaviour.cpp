#include "VortexEngine.hpp"

class LightBehaviour : public VortexMonoBehaviour
{
private:
    VortexModel *target_player = nullptr;
    Vec3 offset = Vec3(0.0f, 5.0f, 0.0f);
public:

    LightBehaviour()
    {
        expose_vec3("offset", &offset, true);
    }

    void on_start() override
    {
        target_player = VortexObjectManager::get_object_by_tag("Player");
    }

    void on_update(float deltaTime) override
    {
        if (target_player) transform().position = target_player->transform.position + offset;
    }
};

VORTEX_REGISTER_SCRIPT(LightBehaviour);
