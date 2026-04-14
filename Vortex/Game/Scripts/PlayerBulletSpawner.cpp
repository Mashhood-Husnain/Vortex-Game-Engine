#include "vortex_behaviour.hpp"
#include "util/vortex_script_registry.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_hud.hpp"

#include "glm/glm.hpp"

class PlayerBulletSpawner : public VortexMonoBehaviour
{
private:
    glm::vec3 bullet_spawn_offset = glm::vec3(0.0f, 1.5f, 0.0f);
    float cool_down = 0.0f;
public:
    void on_update(float deltaTime) override
    {
        cool_down += deltaTime;

        if (VortexMouse::get_button("LEFT") && cool_down >= 0.1f)
        {
            cool_down = 0.0f;
            VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);

            glm::vec3 flat_forward = gameObject->app->camera->front;
            flat_forward.y = 0.0f;
            flat_forward = glm::normalize(flat_forward);

            bullet->transform.position = gameObject->transform.position + bullet_spawn_offset  + flat_forward;

            bullet->transform.scale = glm::vec3(0.1f, 0.1f, 0.1f);

            bullet->transform.rotation.y = gameObject->transform.rotation.y;

            VortexMonoBehaviour *move_script = ScriptRegistry::get().create("PlayerBulletMover");

            bullet->add_behaviour("PlayerBulletMover", move_script);

            VortexObjectManager::pending_models.push_back(bullet);
        }
    }
};

VORTEX_REGISTER_SCRIPT(PlayerBulletSpawner);
