#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"

#include "glm/glm.hpp"

class PlayerBulletSpawner : public VortexMonoBehaviour
{
private:
    glm::vec3 bullet_spawn_offset = glm::vec3(0.0f, 1.5f, 0.0f);
public:
    void on_update(float deltaTime) override
    {
        if (VortexMouse::get_button_down("LEFT"))
        {
            VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);

            glm::vec3 flat_forward = gameObject->app->camera->front;
            flat_forward.y = 0.0f;
            flat_forward = glm::normalize(flat_forward);

            bullet->transform.position = gameObject->transform.position + bullet_spawn_offset  + flat_forward;

            bullet->transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);

            bullet->transform.rotation.y = gameObject->transform.rotation.y;


            VortexMonoBehaviour *move_script = ScriptRegistry::get().create("PlayerBulletMover");

            bullet->show_collider = true;

            bullet->add_behaviour("PlayerBulletMover", move_script);

            gameObject->app->pending_models.push_back(bullet);
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletSpawner);
