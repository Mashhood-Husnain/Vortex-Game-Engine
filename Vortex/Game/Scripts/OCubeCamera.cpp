#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"
#include "util/vortex_keyboard.hpp"
#include "util/vortex_mouse.hpp"

#include "vortex_camera.hpp"
#include <glm/glm.hpp>

class OCubeCamera : public VortexMonoBehaviour
{
private:
    VortexCamera *my_camera = nullptr;
    glm::vec3 head_offset = glm::vec3(0.0f, 1.5f, 0.0f);
public:
    void on_start() override
    {
        my_camera = new VortexCamera();
        my_camera->aspect_ratio = gameObject->app->editor_camera->aspect_ratio;
        gameObject->app->camera = my_camera;
    }

    void on_update(float deltaTime) override
    {
        my_camera->position = gameObject->position + head_offset;

        if (my_camera->pitch >45.0f) my_camera->pitch = 45.0f;
        if (my_camera->pitch < -45.0f) my_camera->pitch = -45.0f;

        my_camera->update_camera_vectors();

        gameObject->rotation.y = -(my_camera->yaw + 90.0f);
    }
};

VORTEX_REGISTER_SCRIPT(OCubeCamera);
