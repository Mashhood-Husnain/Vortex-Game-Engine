#include "VortexEngine.hpp"

class PlayerCamera : public VortexMonoBehaviour
{
private:
    VortexCamera *my_camera = nullptr;
    
    Vec3 first_person_offset = Vec3(0.0f, 1.5f, 0.0f);
    
    // Standard over-the-right-shoulder 3rd person offset
    // X = 0.8 (right), Y = 2.0 (up), Z = 3.5 (distance behind)
    Vec3 third_person_offset = Vec3(0.8f, 2.0f, 3.5f); 
    
    Vec3 current_offset;
    bool is_third_person = false;

    float safe_camera_pitch = 0.0f;
public:
    PlayerCamera()
    {
        expose_vec3("1st Person Offset", &first_person_offset, true);
        expose_vec3("3rd Person Offset", &third_person_offset, true);
        expose_bool("Start in 3rd Person", &is_third_person, true);
        
        expose_float("camera_pitch", &safe_camera_pitch, false);
    }

    void on_start() override
    {
        my_camera = new VortexCamera();
        my_camera->aspect_ratio = gameObject->app->editor_camera->aspect_ratio;

        current_offset = is_third_person ? third_person_offset : first_person_offset;

        gameObject->app->camera = my_camera;
        safe_camera_pitch = my_camera->pitch;
    }

    void late_update(float deltaTime) override
    {
        if (VortexKeyboard::get_key_down("P"))
        {
            is_third_person = !is_third_person;
        }

        Vec3 target_offset = is_third_person ? third_person_offset : first_person_offset;
        bool is_aiming = VortexMouse::get_button("RIGHT");
        if (is_third_person && is_aiming)
        {
            target_offset.z -= 1.5f;
            target_offset.x += 0.3f;
        }

        current_offset = glm::mix(current_offset, target_offset, deltaTime * 12.0f);

        my_camera->pitch += safe_camera_pitch;

        safe_camera_pitch = 0.0f;
        if (my_camera->pitch > 80.0f) my_camera->pitch = 80.0f;
        if (my_camera->pitch < -80.0f) my_camera->pitch = -80.0f;

        my_camera->update_camera_vectors();

        Vec3 scale = gameObject->transform.scale;
        Vec3 scaled_offset = current_offset * scale;

        Vec3 pivot_pos = gameObject->transform.position;
        
        my_camera->position = pivot_pos 
                            - (my_camera->front * scaled_offset.z) 
                            + (Vec3(0.0f, 1.0f, 0.0f) * scaled_offset.y) 
                            + (my_camera->right * scaled_offset.x);

        float player_yaw = -(my_camera->yaw + 90.0f);
        gameObject->transform.set_euler(Vec3(0.0f, player_yaw, 0.0f));
    }
};

VORTEX_REGISTER_SCRIPT(PlayerCamera);
