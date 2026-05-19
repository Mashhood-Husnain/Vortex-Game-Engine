#include "vortex_objectmanager.hpp"
#include "vortex_particlesystem.hpp"

VortexShader *VortexObjectManager::model_shader = nullptr;
VortexShader *VortexObjectManager::collider_shader = nullptr;
VortexShader *VortexObjectManager::particle_shader = nullptr;
VortexShader *VortexObjectManager::unlit_shader = nullptr;

std::vector<VortexModel*> VortexObjectManager::active_models;
std::vector<VortexModel*> VortexObjectManager::pending_models;
std::vector<ParticleSystem*> VortexObjectManager::active_particlesystems;

void VortexObjectManager::init()
{
    particle_shader = new VortexShader("shaders/particles.vert", "shaders/particles.frag");
    model_shader = new VortexShader("shaders/default.vert", "shaders/default.frag");
    collider_shader = new VortexShader("shaders/collider.vert", "shaders/collider.frag");
    unlit_shader = new VortexShader("shaders/unlit.vert", "shaders/unlit.frag");
}

void VortexObjectManager::clean_up()
{
    for (VortexModel* model : active_models)
    {
        delete model;
    }
    active_models.clear();

    for (VortexModel* model : pending_models)
    {
        delete model;
    }
    pending_models.clear();

    for (ParticleSystem* ps : active_particlesystems)
    {
        delete ps;
    }
    active_particlesystems.clear();    
}

void VortexObjectManager::update(float deltaTime)
{
    for (VortexModel *model : active_models)
    {
        if (model->is_active)
        {
            model->update(deltaTime);
            model->late_update(deltaTime);
        }
    }

    if (!pending_models.empty())
    {
        for (VortexModel* new_model : pending_models)
        {
            active_models.push_back(new_model);
            
            for (VortexMonoBehaviour* script : new_model->behaviours)
            {
                script->on_start();
            }
        }
        pending_models.clear(); 
    }

    for (ParticleSystem *ps : active_particlesystems)
    {
        ps->update(deltaTime);
    }
}

void VortexObjectManager::draw(VortexCamera &camera, bool show_wireframe)
{
    Frustum cam_frustum = camera.get_frustum();

    for (VortexModel *model : active_models)
    {
        if (model->is_active)
        {
            std::vector<glm::vec3> min_max = model->get_world_bounds_min_max();

            if (!VortexPhysics::aabb_in_frustum(min_max[0], min_max[1], cam_frustum))
            {
                continue;
            }

            if (model->light)
            {
                unlit_shader->use();
                unlit_shader->setVec3("lightColor", model->light->color * model->light->intensity);
                model->draw(*unlit_shader, camera, show_wireframe);
            }
            else
            {
                model->draw(*model_shader, camera, show_wireframe);
            }

            if (model->show_collider)
            {
                model->shared_data->collider.draw(*collider_shader, camera, model);

                for (size_t i = 0; i < model->shared_data->objects.size(); i++)
                {
                    if (model->active_parts[i])
                    {
                        model->shared_data->objects[i].collider.draw(*collider_shader, camera, model);
                    }
                }
            }
        }
    }

    for (ParticleSystem *ps : active_particlesystems)
    {
        ps->draw(*particle_shader, camera);
    }
}

void VortexObjectManager::check_object_status()
{
    active_particlesystems.erase(
        std::remove_if(
            active_particlesystems.begin(),
            active_particlesystems.end(),
            [](ParticleSystem *ps){
                if (ps->should_destroy)
                {
                    delete ps;
                    return true;
                }
                return false;
            }),
        active_particlesystems.end()
    );

    active_models.erase(
        std::remove_if(
            active_models.begin(),
            active_models.end(),
            [](VortexModel *model){

                if (!model->active_parts.empty())
                {
                    bool any_part_alive = false;
                    for (bool is_alive : model->active_parts)
                    {
                        if (is_alive)
                        {
                            any_part_alive = true;
                            break;
                        }
                    }

                    if (!any_part_alive) model->should_destroy = true;
                }

                if (model->should_destroy)
                {
                    delete model;
                    return true;
                }
                return false;
            }),
        active_models.end()
    );
}

VortexModel* VortexObjectManager::get_object_by_tag(std::string tag)
{
    for (auto *model : active_models)
    {
        if (model->model_name == tag)
        {
            return model;
        }
    }

    return nullptr;
}

void VortexObjectManager::destroy_object(VortexModel *target_object)
{
    target_object->should_destroy = true;
}
