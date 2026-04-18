#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_physics.hpp"
#include "vortex_audio.hpp"
#include "vortex_uimanager.hpp"

#include <glm/glm.hpp>
#include <cstdlib>
#include <string>
#include <algorithm>

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

    int max_mag_capacity = 30;
    int max_bullets = 270;
    int current_mag_bullets;

    bool is_reloading = false;
    float reload_time = 2.0f;
    float reload_timer = 0.0f;

public:
    void on_start() override
    {
        current_offset = hip_offset;
        current_mag_bullets = max_mag_capacity;
        
        setup_hud();
        show_bullet_stats();
    }

    void late_update(float deltaTime) override
    {
        VortexCamera* cam = gameObject->app->camera;
        if (!cam) return;

        if (is_reloading)
        {
            update_reload(deltaTime);
            update_gun_transform(cam, deltaTime); 
            return; 
        }

        if (VortexKeyboard::get_key("R") && current_mag_bullets < max_mag_capacity)
        {
            start_reload();
        }

        if (fire_timer > 0.0f) fire_timer -= deltaTime;

        if (VortexMouse::get_button("LEFT") && fire_timer <= 0.0f)
        {
            fire_weapon(cam);
            fire_timer = fire_rate;
        }

        update_gun_transform(cam, deltaTime);
    }

    void update_gun_transform(VortexCamera* cam, float deltaTime)
    {
        bool is_aiming = VortexMouse::get_button("RIGHT") && !is_reloading;
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
    }

    void fire_weapon(VortexCamera* cam)
    {
        if (current_mag_bullets <= 0)
        {
            start_reload();
            return;
        }

        current_mag_bullets -= 1;
        show_bullet_stats();

        current_recoil = max_recoil;
        VortexAudio::play_sound("assets/audio/gunshot.wav", 0.25f);

        glm::vec3 barrel_tip = gameObject->transform.position + (cam->front * 1.5f);

        VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);
        bullet->model_name = "Bullet";
        bullet->transform.position = barrel_tip;
        bullet->transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);
        
        VortexMonoBehaviour *move_script = ScriptRegistry::get().create("PlayerBulletMover");
        bullet->add_behaviour("PlayerBulletMover", move_script);

        VortexObjectManager::pending_models.push_back(bullet);
    }

    void start_reload()
    {
        if (max_bullets <= 0 || is_reloading) return;

        is_reloading = true;
        reload_timer = 0.0f;

        VortexCanvas* hud = VortexUIManager::get_canvas("Gun_stats_Canvas");
        if (hud)
        {
            VortexCanvas_UIElement* bar = hud->get_element("RELOAD_PROGRESS");
            if (bar) bar->is_active = true;
        }
        VortexAudio::play_sound("assets/audio/reload.mp3", 0.5f);
    }

    void update_reload(float deltaTime)
    {
        reload_timer += deltaTime;

        VortexCanvas* hud = VortexUIManager::get_canvas("Gun_stats_Canvas");
        if (hud)
        {
            VortexCanvas_UIElement* bar = hud->get_element("RELOAD_PROGRESS");
            if (bar) bar->current_value = reload_timer / reload_time;
        }

        if (reload_timer >= reload_time)
        {
            finish_reload();
        }
    }

    void finish_reload()
    {
        is_reloading = false;

        int bullets_needed = max_mag_capacity - current_mag_bullets;
        int bullets_to_add = std::min(bullets_needed, max_bullets);

        current_mag_bullets += bullets_to_add;
        max_bullets -= bullets_to_add;

        VortexCanvas* hud = VortexUIManager::get_canvas("Gun_stats_Canvas");
        if (hud)
        {
            VortexCanvas_UIElement* bar = hud->get_element("RELOAD_PROGRESS");
            if (bar) bar->is_active = false;
        }
        show_bullet_stats();
    }

    void setup_hud()
    {
        VortexCanvas* hud_canvas = new VortexCanvas("Gun_stats_Canvas");

        VortexCanvas_UIElement ammo_text;
        ammo_text.id = "AMMO_DISPLAY";
        ammo_text.type = VortexCanvas_UIType::COUNTER;
        ammo_text.position = VortexHUD::GetAnchorPosition(UIAnchor::BOTTOM_RIGHT, -250.0f, -80.0f);
        ammo_text.text_data = "AMMO";
        ammo_text.current_value = (float)current_mag_bullets;
        ammo_text.color = ImVec4(1, 1, 1, 1);
        hud_canvas->add_element(ammo_text);

        VortexCanvas_UIElement reload_bar;
        reload_bar.id = "RELOAD_PROGRESS";
        reload_bar.type = VortexCanvas_UIType::BAR;
        reload_bar.position = VortexHUD::GetAnchorPosition(UIAnchor::CENTER, -150.0f, 150.0f); 
        reload_bar.size = ImVec2(300.0f, 12.0f);
        reload_bar.text_data = "RELOADING...";
        reload_bar.current_value = 0.0f;
        reload_bar.max_value = 1.0f;
        reload_bar.color = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
        reload_bar.is_active = false;
        hud_canvas->add_element(reload_bar);

        VortexUIManager::add_canvas(hud_canvas);
    }

    void show_bullet_stats()
    {
        VortexCanvas* hud = VortexUIManager::get_canvas("Gun_stats_Canvas");
        if (hud)
        {
            VortexCanvas_UIElement* ammo = hud->get_element("AMMO_DISPLAY");
            if (ammo)
            {
                ammo->current_value = (float)current_mag_bullets;
                ammo->text_data = "AMMO: " + std::to_string(current_mag_bullets) + " / " + std::to_string(max_bullets);
            }
        }
    }
};

VORTEX_REGISTER_SCRIPT(GunBehaviour);
