#include "vortex_objectmanager.hpp"
#include "vortex_particlesystem.hpp"

VortexShader *VortexObjectManager::model_shader = nullptr;
VortexShader *VortexObjectManager::collider_shader = nullptr;
VortexShader *VortexObjectManager::particle_shader = nullptr;
VortexShader *VortexObjectManager::unlit_shader = nullptr;
VortexShader *VortexObjectManager::depth_only_shader = nullptr;
VortexShader *VortexObjectManager::outline_shader = nullptr;

std::vector<VortexModel*> VortexObjectManager::active_models;
std::vector<VortexModel*> VortexObjectManager::pending_models;
std::vector<ParticleSystem*> VortexObjectManager::active_particlesystems;

void setup_occlusion_cube()
{
    if (occlusion_VAO != 0) return;

    glGenVertexArrays(1, &occlusion_VAO);
    glGenBuffers(1, &occlusion_VBO);
    glBindVertexArray(occlusion_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, occlusion_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(GLOBAL::DEFAULT_VERTICES::OCCLUSION_VERTICES),
        GLOBAL::DEFAULT_VERTICES::OCCLUSION_VERTICES,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void VortexObjectManager::init()
{
    particle_shader = new VortexShader("shaders/particles.vert", "shaders/particles.frag");
    model_shader = new VortexShader("shaders/default.vert", "shaders/default.frag");
    collider_shader = new VortexShader("shaders/collider.vert", "shaders/collider.frag");
    unlit_shader = new VortexShader("shaders/unlit.vert", "shaders/unlit.frag");
    depth_only_shader = new VortexShader("shaders/depth_only.vert", "shaders/depth_only.frag");
    outline_shader = new VortexShader("shaders/outline.vert", "shaders/outline.frag");
}

void VortexObjectManager::clean_up()
{
    VORTEX_INFO("[OBJECT MANAGER] Destroying active scene entities and clearing simulation registry...");

    for (VortexModel* model : active_models) delete model;
    active_models.clear();

    for (VortexModel* model : pending_models) delete model;
    pending_models.clear();

    for (ParticleSystem* ps : active_particlesystems) delete ps;
    active_particlesystems.clear();

    delete model_shader;
    model_shader = nullptr;

    delete collider_shader;
    collider_shader = nullptr;

    delete particle_shader;
    particle_shader = nullptr;

    delete unlit_shader;
    unlit_shader = nullptr;

    delete depth_only_shader;
    depth_only_shader = nullptr;
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

void VortexObjectManager::draw(VortexCamera &camera, bool show_wireframe, bool is_render_pass)
{
    Frustum cam_frustum = camera.get_frustum();
    glm::vec3 camera_pos = camera.get_position();

    std::vector<VortexModel*> render_queue;
    for (VortexModel *model : active_models)
    {
        if (model->is_active) render_queue.push_back(model);
    }

    std::sort(render_queue.begin(), render_queue.end(), [&camera_pos](VortexModel *a, VortexModel *b){
        glm::vec3 da = a->transform.position - camera_pos;
        glm::vec3 db = b->transform.position - camera_pos;

        float dist_a = glm::dot(da, da);
        float dist_b = glm::dot(db, db);

        return dist_a < dist_b;
    });

    setup_occlusion_cube();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDepthMask(GL_TRUE);

    for (VortexModel *model : render_queue)
    {
        if (!model->is_active) continue;

        std::vector<glm::vec3> min_max = model->get_world_bounds_min_max();
        glm::vec3 min_bound = min_max[0];
        glm::vec3 max_bound = min_max[1];

        bool is_camera_inside = (camera_pos.x >= min_bound.x && camera_pos.x <= max_bound.x &&
                                 camera_pos.y >= min_bound.y && camera_pos.y <= max_bound.y &&
                                 camera_pos.z >= min_bound.z && camera_pos.z <= max_bound.z);

        if (!is_camera_inside && !VortexPhysics::aabb_in_frustum(min_bound, max_bound, cam_frustum))
        {
            model->is_visible = true;
            continue;
        }

        GLuint available = 0;
        glGetQueryObjectuiv(model->occlusion_query, GL_QUERY_RESULT_AVAILABLE, &available);

        if (available)
        {
            GLuint passed = 0;
            glGetQueryObjectuiv(model->occlusion_query, GL_QUERY_RESULT, &passed);

            model->is_visible = (passed > 0);
        }

        if (model->is_visible || is_camera_inside)
        {
            if(model->is_selected)
            {
                glEnable(GL_STENCIL_TEST);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);
            }

            if (model->light)
            {
                unlit_shader->use();
                unlit_shader->setVec3("lightColor", model->light->color * model->light->intensity);
                model->draw(*unlit_shader, camera, show_wireframe);
            }
            else
            {
                model->draw(*model_shader, camera, show_wireframe, is_render_pass);
            }

            if (model->is_selected)
            {
                glStencilMask(0x00);
                glDisable(GL_STENCIL_TEST);
            }

            if (model->show_collider)
            {
                glDisable(GL_CULL_FACE);

                model->shared_data->collider.draw(*collider_shader, camera, model);

                for (size_t i = 0; i < model->shared_data->objects.size(); i++)
                {
                    if (model->active_parts[i])
                    {
                        model->shared_data->objects[i].collider.draw(*collider_shader, camera, model);
                    }
                }

                glEnable(GL_CULL_FACE);
            }
        }

        if (!is_camera_inside)
        {
            glm::vec3 center = (min_bound + max_bound) * 0.5f;
            glm::vec3 size = (max_bound - min_bound) * 1.02f;
            glm::mat4 box_model = glm::translate(glm::mat4(1.0f), center);
            box_model = glm::scale(box_model, size);

            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL);

            depth_only_shader->use();
            depth_only_shader->setMat4("view", camera.getViewMatrix());
            depth_only_shader->setMat4("projection", camera.getProjectionMatrix());
            depth_only_shader->setMat4("model", box_model);

            glBeginQuery(GL_ANY_SAMPLES_PASSED, model->occlusion_query);
            glBindVertexArray(occlusion_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glEndQuery(GL_ANY_SAMPLES_PASSED);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
        }
    }

    if (!is_render_pass)
    {
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        outline_shader->use();
        outline_shader->setMat4("view", camera.getViewMatrix());
        outline_shader->setMat4("projection", camera.getProjectionMatrix());

        outline_shader->setVec3("outline_color", glm::vec3(1.0f, 0.6f, 0.0f));
        outline_shader->setFloat("outline_thickness", 0.05f);

        for (VortexModel *model : active_models)
        {
            if (model->is_active && model->is_selected)
            {
                model->draw(*outline_shader, camera, false);
            }
        }

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }

    glDisable(GL_CULL_FACE);

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

void VortexObjectManager::clear_scene()
{
    VORTEX_INFO("[OBJECT MANAGER] Wiping scene memory for context switch...");

    for (VortexModel* model : active_models)
    {
        delete model;
    }
    active_models.clear();

    for (ParticleSystem* ps : active_particlesystems)
    {
        delete ps;
    }
    active_particlesystems.clear();

    VortexAssetManager::clean_up();
}
