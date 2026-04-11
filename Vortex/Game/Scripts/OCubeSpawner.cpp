#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"
#include "util/vortex_keyboard.hpp"
#include "util/vortex_mouse.hpp"

#include "glm/glm.hpp"

class OCubeSpawner : public VortexMonoBehaviour
{
private:
public:
    void on_update(float deltaTime) override
    {
        if (VortexMouse::get_button_down("LEFT"))
        {
            VortexModel *bullet = new VortexModel("assets/models/obj/cube.obj", gameObject->app);

            glm::vec3 flat_forward = gameObject->app->camera->front;
            flat_forward.y = 0.0f;
            flat_forward = glm::normalize(flat_forward);

            bullet->position = gameObject->position + (flat_forward);

            bullet->scale = glm::vec3(0.1f, 0.1f, 1.0f);

            bullet->rotation.y = gameObject->rotation.y;


            VortexMonoBehaviour *move_script = ScriptRegistry::get().create("OCubeMover");
            bullet->add_behaviour("OCubeMover", move_script);

            gameObject->app->pending_models.push_back(bullet);
        }
    }
};

VORTEX_REGISTER_SCRIPT(OCubeSpawner);
