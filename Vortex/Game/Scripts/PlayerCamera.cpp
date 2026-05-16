#include "VortexEngine.hpp"

class PlayerCamera : public VortexMonoBehaviour
{
private:
    VortexModel *player_target = nullptr;
    VortexCamera *player_camera = nullptr;

    Vec3 current_offset = Vec3(0.0f, 1.5f, 0.0f);
public:
    void on_start() override
    {
        player_target = VortexObjectManager::get_object_by_tag("Player");
        
        player_camera = new VortexCamera();
        player_camera->set_aspect_ratio(engine().get_camera()->get_aspect_ratio());

        engine().set_camera(player_camera);
    }

    void on_update(float deltaTime) override
    {
        float new_cam_pitch = player_camera->get_pitch();
        if (new_cam_pitch > 80.0f) new_cam_pitch = 80.0f;
        if (new_cam_pitch < -80.0f) new_cam_pitch = -80.0f;

        player_camera->set_rotation(player_camera->get_yaw(), new_cam_pitch);

        Vec3 scale = transform().scale;
        Vec3 scaled_offset = current_offset * scale;
        Vec3 pivot_pos = transform().position;
        Vec3 new_camera_position = pivot_pos -
                                   (player_camera->get_front() * scaled_offset.z) +
                                   (Vec3(0.0f, 1.0f, 0.0f) * scaled_offset.y) +
                                   (player_camera->get_right() * scaled_offset.x);

        player_camera->set_position(new_camera_position);
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

VORTEX_REGISTER_SCRIPT(PlayerCamera);
