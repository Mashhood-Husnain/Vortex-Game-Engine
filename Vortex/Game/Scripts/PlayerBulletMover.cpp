#include "VortexEngine.hpp"

#include "myTags.hpp"

class PlayerBulletMover : public VortexMonoBehaviour
{
private:
    float speed = 150.0f;
    float distance_traveled = 0.0f;
    Vec3 fly_direction;
    float bullet_damage = 5.0f;
    VortexModel *player;
    VortexModel *gun;
public:
    void on_start() override
    {
        player = VortexObjectManager::get_object_by_tag("Player");
        gun = VortexObjectManager::get_object_by_tag("Gun");

        VortexCamera* cam = engine().get_camera();

        Vec3 crosshair_target = cam->get_position() + (cam->get_front() * 100.0f);
        
        Vec3 base_direction = glm::normalize(crosshair_target - transform().position);

        bool is_aiming = VortexMouse::get_button("RIGHT");
        float spread_factor = is_aiming ? 0.005f : 0.05f; 

        Vec3 spread(
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor,
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor,
            ((rand() / (float)RAND_MAX) - 0.5f) * spread_factor
        );

        fly_direction = glm::normalize(base_direction + spread);
    }

    void on_update(float deltaTime) override
    {
        float move_dist = speed * deltaTime;

        RaycastHit hit = VortexPhysics::raycast(transform().position, fly_direction, move_dist, {&object(), player, gun});

        if (hit.has_hit)
        {
            transform().position = hit.hit_point;

            if (hit.hit_model->get_componant<DestructibleTag>() && hit.hit_sub_object_index != -1)
            {
                VortexAudio::play_sound("assets/audio/gunshot_hit.wav", 0.5f);
                hit.hit_model->active_parts[hit.hit_sub_object_index] = false;
            }

            if (hit.hit_model->get_componant<EnemyTag>())
            {
                VortexAudio::play_sound("assets/audio/gunshot_hit.wav", 0.5f);
                hit.hit_model->send_message("TAKE_DAMAGE", &bullet_damage);
            }

            object().should_destroy = true;
            return;
        }

        transform().position += fly_direction * move_dist;
        distance_traveled += move_dist;

        if (distance_traveled >= 200.0f)
        {
            object().should_destroy = true;
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletMover);
