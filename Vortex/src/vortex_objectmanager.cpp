#include "vortex_objectmanager.hpp"

VortexShader *VortexObjectManager::model_shader = nullptr;
VortexShader *VortexObjectManager::collider_shader = nullptr;
VortexShader *VortexObjectManager::particle_shader = nullptr;

std::vector<VortexModel*> VortexObjectManager::active_models;
std::vector<VortexModel*> VortexObjectManager::pending_models;
std::vector<ParticleSystem*> VortexObjectManager::active_particlesystems;

void VortexObjectManager::init()
{
    particle_shader = new VortexShader("shaders/particles.vert", "shaders/particles.frag");
    model_shader = new VortexShader("shaders/default.vert", "shaders/default.frag");
    collider_shader = new VortexShader("shaders/collider.vert", "shaders/collider.frag");
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
        }
    }

    for (VortexModel *model : active_models)
    {
        if (model->is_active)
        {
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
    for (VortexModel *model : active_models)
    {
        if (model->is_active)
        {
            model->draw(*model_shader, camera, show_wireframe);
            
            if (model->show_collider)
            {
                model->shared_data->collider.draw(*collider_shader, camera, model);

                for (auto& obj : model->shared_data->objects)
                {
                    obj.collider.draw(*collider_shader, camera, model);
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
