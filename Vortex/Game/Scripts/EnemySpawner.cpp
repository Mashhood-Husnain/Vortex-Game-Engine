#include "VortexEngine.hpp"

#include "myTags.hpp"

class EnemySpawner : public VortexMonoBehaviour
{
private:
    float spawn_timer = 0.0f;
    float spawn_interval = 5.0f;
    
    int enemies_per_wave = 5;
    float min_radius = 40.0f;
    float max_radius = 60.0f;
public:
    void on_start() override
    {
        spawn_timer = 0.0f;
    }

    void on_update(float deltaTime) override
    {
        spawn_timer += deltaTime;

        if (spawn_timer >= spawn_interval)
        {
            spawn_timer = 0.0f;
            
            for (int i = 0; i < enemies_per_wave; i++) 
            {
                VortexModel *enemy = new VortexModel("assets/models/obj/capsule.obj", gameObject->app);
                
                float random_angle = static_cast<float>(rand()) / RAND_MAX * (2.0f * 3.14159f);
                
                float random_radius = min_radius + static_cast<float>(rand()) / RAND_MAX * (max_radius - min_radius);
                
                Vec3 player_pos = gameObject->transform.position;
                float spawn_x = player_pos.x + (cos(random_angle) * random_radius);
                float spawn_z = player_pos.z + (sin(random_angle) * random_radius);
                
                enemy->transform.position = Vec3(spawn_x, 0.0f, spawn_z);
                enemy->transform.scale = Vec3(2.0f, 2.0f, 2.0f);

                enemy->model_name = "Enemy";
                
                VortexMonoBehaviour *enemy_script = ScriptRegistry::get().create("EnemyMovement");
                VortexRigidbody *rigidbody = new VortexRigidbody();

                enemy->add_behaviour("EnemyMovement", enemy_script);

                rigidbody->gravity = true;
                enemy->add_behaviour("VortexRigidbody", rigidbody);
                enemy->add_behaviour("EnemyTag", new EnemyTag());
                
                VortexObjectManager::pending_models.push_back(enemy);
            }
        }
    }
};

VORTEX_REGISTER_SCRIPT(EnemySpawner);
