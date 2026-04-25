#include "VortexEngine.hpp"

class GunBehaviour : public VortexMonoBehaviour
{
private:
    float model_yaw_offset = 90.0f;

    Vec3 hip_offset = Vec3(0.380044f, -0.264941f, 0.457343f);
    Vec3 ads_offset = Vec3(0.00759287f, -0.0500511f, -0.207612f);
    Vec3 current_offset;

    float recoil_up_movement = 0.25f;

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

    float current_spread = 0.0f;
    float hip_spread = 25.0f;
    float ads_spread = 5.0f;
    float recoil_spread_mult = 150.0f;

    VortexModel *target_player;
    Vec3 follow_position = Vec3(0.0f);
public:
    GunBehaviour()
    {
        expose_float("recoil_up_movement", &recoil_up_movement, true);
    }

    void on_start() override
    {
        current_offset = hip_offset;
        current_mag_bullets = max_mag_capacity;

        target_player = VortexObjectManager::get_object_by_tag("Player");
        
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
            draw_crosshair(deltaTime, false);
            return; 
        }

        if (VortexKeyboard::get_key("R") && current_mag_bullets < max_mag_capacity)
        {
            start_reload();
        }

        if (fire_timer > 0.0f) fire_timer -= deltaTime;

        if (VortexMouse::get_button("LEFT") && fire_timer <= 0.0f)
        {
            fire_weapon(cam, deltaTime);
            fire_timer = fire_rate;
        }

        update_gun_transform(cam, deltaTime);

        bool is_aiming = VortexMouse::get_button("RIGHT") && !is_reloading;
        draw_crosshair(deltaTime, is_aiming);
    }

    void update_gun_transform(VortexCamera* cam, float deltaTime)
    {
        bool is_aiming = VortexMouse::get_button("RIGHT") && !is_reloading;
        Vec3 target_offset = is_aiming ? ads_offset : hip_offset;

        current_offset = glm::mix(current_offset, target_offset, deltaTime * 15.0f);

        if (current_recoil > 0.0f)
        {
            current_recoil -= recoil_recovery_speed * deltaTime;
            if (current_recoil < 0.0f) current_recoil = 0.0f;
        }

        Vec3 world_up = Vec3(0.0f, 1.0f, 0.0f);
        Vec3 forward = glm::normalize(cam->front);
        Vec3 right   = glm::normalize(glm::cross(forward, world_up));
        Vec3 up      = glm::normalize(glm::cross(right, forward));

        Vec3 scale = gameObject->transform.scale;
        Vec3 scaled_offset = current_offset * scale;

        if (target_player)
        {
            VortexMonoBehaviour *cam_script = target_player->get_behaviour("PlayerCamera");
            if (cam_script)
            {
                bool *is3rdperson = cam_script->search_variable_by_name<bool>("Start in 3rd Person", ScriptVarType::BOOL);
                if (is3rdperson)
                {
                    if (*is3rdperson)
                    {
                        follow_position = target_player->transform.position;
                    }
                    else
                    {
                        follow_position = cam->position;
                    }
                }
            }
        }

        Vec3 hand_pos = follow_position 
                        + (forward * scaled_offset.z) 
                        + (up * scaled_offset.y)
                        + (right * scaled_offset.x);

        hand_pos += (up * current_recoil * 0.3f * scale.y) - (forward * current_recoil * 0.8f * scale.z);

        Vec3 gun_center_pos = hand_pos 
                            + (forward * 0.6f * scale.z)
                            + (up * -0.1f * scale.y);

        gameObject->transform.position = gun_center_pos;

        Mat4 rot = Mat4(1.0f);

        rot[0] = Vec4(right, 0.0f);
        rot[1] = Vec4(up, 0.0f);
        rot[2] = Vec4(-forward, 0.0f);

        if (model_yaw_offset != 0.0f)
        {
            rot = rot * glm::rotate(Mat4(1.0f), glm::radians(model_yaw_offset), Vec3(0, 1, 0));
        }

        gameObject->transform.orientation = Quaternion(rot);
    }

    void fire_weapon(VortexCamera* cam, float deltaTime)
    {
        if (current_mag_bullets <= 0)
        {
            start_reload();
            return;
        }

        if (target_player)
        {
            VortexMonoBehaviour *cam_script = target_player->get_behaviour("PlayerCamera");
            if (cam_script)
            {
                float *cam_pitch = cam_script->search_variable_by_name<float>("camera_pitch", ScriptVarType::FLOAT);
                if (cam_pitch)
                {
                    *cam_pitch += recoil_up_movement;
                }
            }
        }

        current_mag_bullets -= 1;
        show_bullet_stats();

        current_recoil = max_recoil;
        VortexAudio::play_sound("assets/audio/gunshot.wav", 0.25f);

        Vec3 scale = gameObject->transform.scale;
        Vec3 barrel_tip = gameObject->transform.position + (cam->front * 1.5f * scale.z);

        VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);
        bullet->model_name = "Bullet";
        bullet->transform.position = barrel_tip;
        bullet->transform.scale = Vec3(0.1f, 0.1f, 0.1f);
        
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

    void draw_crosshair(float deltaTime, bool is_aiming)
    {
        float target_base_spread = is_aiming ? ads_spread : hip_spread;        
        float dynamic_spread = target_base_spread + (current_recoil * recoil_spread_mult);

        current_spread = glm::mix(current_spread, dynamic_spread, deltaTime * 20.0f);

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImDrawList* draw_list = ImGui::GetForegroundDrawList(ImGui::GetMainViewport());

        float line_length = 12.0f;
        float thickness = 2.0f;
        
        float alpha = is_aiming ? 0.3f : 1.0f; 
        ImU32 color = IM_COL32(0, 255, 0, (int)(255 * alpha));

        draw_list->AddLine(
            ImVec2(center.x - current_spread - line_length, center.y), 
            ImVec2(center.x - current_spread, center.y), 
            color, thickness);
            
        draw_list->AddLine(
            ImVec2(center.x + current_spread, center.y), 
            ImVec2(center.x + current_spread + line_length, center.y), 
            color, thickness);
            
        draw_list->AddLine(
            ImVec2(center.x, center.y - current_spread - line_length), 
            ImVec2(center.x, center.y - current_spread), 
            color, thickness);
            
        draw_list->AddLine(
            ImVec2(center.x, center.y + current_spread), 
            ImVec2(center.x, center.y + current_spread + line_length), 
            color, thickness);

        draw_list->AddCircleFilled(center, 2.0f, color);
    }
};

VORTEX_REGISTER_SCRIPT(GunBehaviour);
