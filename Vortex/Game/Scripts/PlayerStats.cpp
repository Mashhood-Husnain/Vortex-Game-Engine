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

class PlayerStats : public VortexMonoBehaviour
{
private:
    float points = 0;
    float health = 100.0f;
    float damage_cooldown = 0.0f;
public:

    void on_start() override
    {
        setup_hud();
        sync_hud();
    }

    void on_update(float deltaTime) override
    {
        damage_cooldown += deltaTime;
        sync_hud();
    }

    void on_message(const std::string &message, void *data) override
    {
        VortexModel* canvas_model = VortexObjectManager::get_object_by_tag("Canvas");

        if (message == "TAKE_DAMAGE" && data != nullptr)
        {
            if (damage_cooldown >= 1.0f)
            {
                float damage = *static_cast<float*>(data);
                health -= damage;

                damage_cooldown = 0.0f;

                VortexAudio::play_sound("assets/audio/player_damage.wav", 1.0f);

                if (canvas_model)
                {
                    std::pair<std::string, float> ui_update("PLAYER_HEALTH", health);
                    canvas_model->send_message("UPDATE_UI_VALUE", &ui_update);
                }
            }
        }
        else if (message == "ADD_SCORE" && data != nullptr)
        {
            float new_points = *static_cast<float*>(data);

            points += new_points;

            if (canvas_model)
            {
                std::pair<std::string, float> ui_update("PLAYER_SCORE", points);
                canvas_model->send_message("UPDATE_UI_VALUE", &ui_update);
            }
        }
    }

    void setup_hud()
    {
        VortexCanvas *hud_canvas = new VortexCanvas("Player_stats_Canvas");

        VortexCanvas_UIElement health_bar;
        health_bar.id = "PLAYER_HEALTH";
        health_bar.type = VortexCanvas_UIType::BAR;
        health_bar.position = VortexHUD::GetAnchorPosition(UIAnchor::TOP_LEFT, 20.0f, 20.0f);
        health_bar.size = ImVec2(300.0f, 25.0f);
        health_bar.text_data = "HP";
        health_bar.current_value = 100.0f;
        health_bar.max_value = 100.0f;
        health_bar.color = ImVec4(0.8f, 0.1f, 0.1f, 1.0f);
        hud_canvas->add_element(health_bar);

        VortexCanvas_UIElement score_text;
        score_text.id = "PLAYER_SCORE";
        score_text.type = VortexCanvas_UIType::COUNTER;
        score_text.position = VortexHUD::GetAnchorPosition(UIAnchor::TOP_CENTER, -50.0f, 20.0f);
        score_text.text_data = "SCORE";
        score_text.current_value = points;
        score_text.color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        hud_canvas->add_element(score_text);

        VortexUIManager::add_canvas(hud_canvas);
    }

    void sync_hud()
    {
        VortexCanvas* hud = VortexUIManager::get_canvas("Player_stats_Canvas");
        if (hud)
        {
            VortexCanvas_UIElement* hp_bar = hud->get_element("PLAYER_HEALTH");
            if (hp_bar) hp_bar->current_value = health;

            VortexCanvas_UIElement* score_el = hud->get_element("PLAYER_SCORE");
            if (score_el) score_el->current_value = points;
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerStats);
