#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"
#include "vortex_physics.hpp"
#include "vortex_audio.hpp"

class PlayerStats : public VortexMonoBehaviour
{
private:
    float points = 0;
    float health = 100.0f;
    float damage_cooldown = 0.0f;
public:

    void on_update(float deltaTime) override
    {
        damage_cooldown += deltaTime;
        draw_hud();
    }

    void on_message(const std::string &message, void *data) override
    {
        if (message == "TAKE_DAMAGE" && data != nullptr)
        {
            if (damage_cooldown >= 1.0f)
            {
                float damage = *static_cast<float*>(data);
                health -= damage;

                damage_cooldown = 0.0f;

                VortexAudio::play_sound("assets/audio/player_damage.wav", 1.0f);
            }
        }
        else if (message == "ADD_SCORE" && data != nullptr)
        {
            float new_points = *static_cast<float*>(data);

            points += new_points;
        }
    }

    void draw_hud()
    {
        VortexHUD::Begin();

        VortexHUD::Bar("HEALTH", health, 100.0f, ImVec2(50, 50), ImVec2(300, 20), ImVec4(1, 0, 0, 1));
        VortexHUD::Counter("SCORE", points, ImVec2(800, 50), ImVec4(1, 1, 0, 1));

        VortexHUD::End();
    }
};

VORTEX_REGISTER_SCRIPT(PlayerStats);
