#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_physics.hpp"
#include "vortex_audio.hpp"

#include <glm/glm.hpp>
#include <cstdlib>

class GunBehaviour : public VortexMonoBehaviour
{
private:
    float model_yaw_offset = 90.0f;

    glm::vec3 hip_offset = glm::vec3(0.5f, -0.4f, 1.0f);
    glm::vec3 ads_offset = glm::vec3(0.0f, -0.3f, 0.8f);
    glm::vec3 current_offset;

    float current_recoil = 0.0f;
    float max_recoil = 0.2f;
    float recoil_recovery_speed = 5.0f;

    float fire_rate = 0.05f;
    float fire_timer = 0.0f;
public:
    void on_start() override
    {
        current_offset = hip_offset;
    }

    void late_update(float deltaTime) override
    {
        VortexCamera* cam = gameObject->app->camera;
        if (!cam) return;

        bool is_aiming = VortexMouse::get_button("RIGHT");
        glm::vec3 target_offset = is_aiming ? ads_offset : hip_offset;

        current_offset = glm::mix(current_offset, target_offset, deltaTime * 15.0f);

        if (current_recoil > 0.0f)
        {
            current_recoil -= recoil_recovery_speed * deltaTime;
            if (current_recoil < 0.0f) current_recoil = 0.0f;
        }

        glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 true_right = glm::normalize(glm::cross(cam->front, world_up));
        glm::vec3 true_up = glm::normalize(glm::cross(true_right, cam->front));
        
        glm::vec3 gun_pos = cam->position 
                          + (cam->front * current_offset.z) 
                          + (true_up * current_offset.y)
                          + (true_right * current_offset.x);

        gun_pos += (true_up * current_recoil * 0.3f) - (cam->front * current_recoil * 0.8f);
        
        gameObject->transform.position = gun_pos;

        gameObject->transform.rotation.y = -(cam->yaw + 90.0f) + model_yaw_offset;

        gameObject->transform.rotation.z = cam->pitch; 

        gameObject->transform.rotation.x = 0.0f; 

        if (fire_timer > 0.0f) fire_timer -= deltaTime;

        if (VortexMouse::get_button("LEFT") && fire_timer <= 0.0f)
        {
            fire_weapon(cam, gun_pos);
            fire_timer = fire_rate;
        }
    }

    void fire_weapon(VortexCamera* cam, glm::vec3 gun_pos)
    {
        current_recoil = max_recoil;
        VortexAudio::play_sound("assets/audio/gunshot.wav", 0.25f);

        glm::vec3 right = glm::normalize(glm::cross(cam->front, cam->up));
        glm::vec3 barrel_tip = gun_pos + (cam->front * 1.5f) + (cam->up * 0.1f);

        VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);

        bullet->model_name = "Bullet";

        bullet->transform.position = barrel_tip;
        bullet->transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);
        bullet->transform.rotation.y = gameObject->transform.rotation.y;

        VortexMonoBehaviour *move_script = ScriptRegistry::get().create("PlayerBulletMover");
        bullet->add_behaviour("PlayerBulletMover", move_script);
        bullet->show_collider = true;

        VortexObjectManager::pending_models.push_back(bullet);
    }
};

VORTEX_REGISTER_SCRIPT(GunBehaviour);
