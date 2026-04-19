#include "VortexEngine.hpp"

class PlayerCamera : public VortexMonoBehaviour
{
private:
    VortexCamera *my_camera = nullptr;
    Vec3 head_offset = Vec3(0.0f, 1.5f, 0.0f);
    Vec3 scaled_offset;
public:
    void on_start() override
    {
        my_camera = new VortexCamera();
        my_camera->aspect_ratio = gameObject->app->editor_camera->aspect_ratio;

        scaled_offset = head_offset * gameObject->transform.scale;
        my_camera->position = gameObject->transform.position + scaled_offset;
        my_camera->look_at(Vec3(0.0f, 1.0f, 0.0f));

        gameObject->app->camera = my_camera;
    }

    void late_update(float deltaTime) override
    {
        my_camera->position = gameObject->transform.position + scaled_offset;

        if (my_camera->pitch >45.0f) my_camera->pitch = 45.0f;
        if (my_camera->pitch < -45.0f) my_camera->pitch = -45.0f;

        my_camera->update_camera_vectors();

        float player_yaw = -(my_camera->yaw + 90.0f);
        gameObject->transform.set_euler(Vec3(0.0f, player_yaw, 0.0f));
    }
};

VORTEX_REGISTER_SCRIPT(PlayerCamera);
