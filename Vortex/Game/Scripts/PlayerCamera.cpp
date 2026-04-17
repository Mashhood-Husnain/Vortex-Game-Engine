#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_audio.hpp"

#include "vortex_camera.hpp"
#include <glm/glm.hpp>

class PlayerCamera : public VortexMonoBehaviour
{
private:
    VortexCamera *my_camera = nullptr;
    glm::vec3 head_offset = glm::vec3(0.0f, 1.5f, 0.0f);
public:
    void on_start() override
    {
        my_camera = new VortexCamera();
        my_camera->aspect_ratio = gameObject->app->editor_camera->aspect_ratio;
        my_camera->position = gameObject->transform.position + head_offset;
        my_camera->look_at(glm::vec3(0.0f, 1.0f, 0.0f));

        gameObject->app->camera = my_camera;
    }

    void late_update(float deltaTime) override
    {
        my_camera->position = gameObject->transform.position + head_offset;

        if (my_camera->pitch >45.0f) my_camera->pitch = 45.0f;
        if (my_camera->pitch < -45.0f) my_camera->pitch = -45.0f;

        my_camera->update_camera_vectors();

        gameObject->transform.rotation.y = -(my_camera->yaw + 90.0f);
    }
};

VORTEX_REGISTER_SCRIPT(PlayerCamera);
