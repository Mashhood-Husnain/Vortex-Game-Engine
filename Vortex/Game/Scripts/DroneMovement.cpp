#include "VortexEngine.hpp"

class DroneMovement : public VortexMonoBehaviour
{
private:
    const float movement_speed = 10.0f;
    // stop and attack in this radius
    const float attack_radius = 5.0f;
    // chase the player in tis radius
    const float search_radius = 10.0f;

    const float laser_shot_cooldown_time = 1.7f;
    float laser_shot_timer = 0.0f;

    VortexModel *target_player = nullptr;
public:
    void on_start() override
    {
        target_player = VortexObjectManager::get_object_by_tag("Player");
    }

    void on_update(float deltaTime) override
    {
        float dist = distance(transform().position, target_player->transform.position);
        Vec3 direction = normalize(target_player->transform.position - transform().position);

        if (laser_shot_timer <= 0.0f)
        {
            if (dist < search_radius && dist > attack_radius)
            {
                transform().position += direction * movement_speed * deltaTime;
            }
            else if (dist <= attack_radius && laser_shot_timer <= 0.0f)
            {
                dist = distance(transform().position, target_player->transform.position);
                
                shoot_laser(dist, direction);
                laser_shot_timer = laser_shot_cooldown_time;
            }
        }

        if (laser_shot_timer > 0.0f) laser_shot_timer -= deltaTime;

        VortexDebugRenderer::get().draw_sphere(transform().position, attack_radius, Vec3(1.0f, 0.0f, 0.0f));
        VortexDebugRenderer::get().draw_sphere(transform().position, search_radius, Vec3(0.0f, 1.0f, 0.0f));
    }

    void shoot_laser(float distance_to_target, Vec3 direction)
    {
        VortexModel *laser = new VortexModel("assets/models/obj/cube.obj", &engine());
        laser->model_name = "Laser_shot";

        laser->show_collider = true;
        
        laser->transform.position = transform().position + direction * ((distance_to_target / 2.0f));
        laser->transform.scale = Vec3(0.10, distance_to_target / 2.0f, 0.10f);

        Vec3 default_up(0.0f, 1.0f, 0.0f);
        Quaternion rot = rotation(default_up, direction);

        laser->transform.orientation = rot;
        
        VortexMonoBehaviour *laser_behaviour_script = ScriptRegistry::get().create("DroneLaserBehaviour");
        laser->add_behaviour("DroneLaserBehaviour", laser_behaviour_script);

        VortexObjectManager::pending_models.push_back(laser);
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

VORTEX_REGISTER_SCRIPT(DroneMovement);
