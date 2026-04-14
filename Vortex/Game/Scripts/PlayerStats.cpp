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
    void on_start() override
    {
        VortexMonoBehaviour_set_value("player_health", health);
        VortexMonoBehaviour_set_value("player_points", points);
        VortexMonoBehaviour_set_value("player_damage_cooldown", damage_cooldown);
    }

    void on_update(float deltaTime) override
    {
        health = VortexMonoBehaviour_get_value("player_health");
        points = VortexMonoBehaviour_get_value("player_points");
        damage_cooldown = VortexMonoBehaviour_get_value("player_damage_cooldown");

        damage_cooldown += deltaTime;
        VortexMonoBehaviour_set_value("player_damage_cooldown", damage_cooldown);

        draw_hud();
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
