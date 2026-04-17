#include "vortex_physics.hpp"

static bool test_AABB_overlap(const WorldAABB& a, const WorldAABB& b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool VortexPhysics::check_collision(VortexModel *modela, VortexModel *modelb)
{
    return check_collision_detailed(modela, modelb).has_hit;
}

CollisionHit VortexPhysics::check_collision_detailed(VortexModel *modela, VortexModel *modelb)
{
    CollisionHit result;
    if (!modela || !modelb || !modela->shared_data || !modelb->shared_data) return result;

    glm::vec3 scale_a = modela->transform.scale * modela->collider_scale;
    glm::vec3 scale_b = modelb->transform.scale * modelb->collider_scale;
    float padding = 0.05f;

    WorldAABB global_a, global_b;
    global_a.min = modela->transform.position + (modela->shared_data->collider.min * scale_a) - padding;
    global_a.max = modela->transform.position + (modela->shared_data->collider.max * scale_a) + padding;
    global_b.min = modelb->transform.position + (modelb->shared_data->collider.min * scale_b) - padding;
    global_b.max = modelb->transform.position + (modelb->shared_data->collider.max * scale_b) + padding;

    if (!test_AABB_overlap(global_a, global_b)) return result;

    for (size_t i = 0; i < modela->shared_data->objects.size(); i++)
    {
        const auto& obj_a = modela->shared_data->objects[i];
        if (!obj_a.is_active) continue;

        WorldAABB sub_a;
        sub_a.min = modela->transform.position + (obj_a.collider.min * scale_a) - padding;
        sub_a.max = modela->transform.position + (obj_a.collider.max * scale_a) + padding;

        for (size_t j = 0; j < modelb->shared_data->objects.size(); j++)
        {
            const auto& obj_b = modelb->shared_data->objects[j];
            if (!obj_b.is_active) continue;

            WorldAABB sub_b;
            sub_b.min = modelb->transform.position + (obj_b.collider.min * scale_b) - padding;
            sub_b.max = modelb->transform.position + (obj_b.collider.max * scale_b) + padding;

            if (test_AABB_overlap(sub_a, sub_b))
            {
                result.has_hit = true;
                result.hit_sub_object_index = (int)j; 
                return result;
            }
        }
    }
    return result;
}

static bool ray_intersect_aabb(glm::vec3 ray_origin, glm::vec3 ray_dir, WorldAABB box, float& t_hit)
{
    glm::vec3 inv_dir = 1.0f / (ray_dir + glm::vec3(1e-6f)); 

    glm::vec3 t0 = (box.min - ray_origin) * inv_dir;
    glm::vec3 t1 = (box.max - ray_origin) * inv_dir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tnear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tfar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (tnear > tfar || tfar < 0.0f) return false;

    t_hit = tnear;
    return true;
}

RaycastHit VortexPhysics::raycast(glm::vec3 origin, glm::vec3 direction, float max_distance)
{
    RaycastHit closest_hit;
    closest_hit.distance = max_distance;

    glm::vec3 dir_norm = glm::normalize(direction);

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        if (!model->is_active || model->should_destroy || !model->shared_data) continue;

        glm::vec3 scale = model->transform.scale * model->collider_scale;
        float padding = 0.05f;

        WorldAABB global_box;
        global_box.min = model->transform.position + (model->shared_data->collider.min * scale) - padding;
        global_box.max = model->transform.position + (model->shared_data->collider.max * scale) + padding;

        float global_t = 0.0f;
        if (!ray_intersect_aabb(origin, dir_norm, global_box, global_t) || global_t > closest_hit.distance)
        {
            continue;
        }

        for (size_t i = 0; i < model->shared_data->objects.size(); i++)
        {
            const auto& obj = model->shared_data->objects[i];
            if (!obj.is_active) continue;

            WorldAABB sub_box;
            sub_box.min = model->transform.position + (obj.collider.min * scale) - padding;
            sub_box.max = model->transform.position + (obj.collider.max * scale) + padding;

            float sub_t = 0.0f;
            if (ray_intersect_aabb(origin, dir_norm, sub_box, sub_t))
            {
                if (sub_t >= 0.0f && sub_t < closest_hit.distance)
                {
                    closest_hit.has_hit = true;
                    closest_hit.hit_model = model;
                    closest_hit.hit_sub_object_index = (int)i;
                    closest_hit.distance = sub_t;
                    closest_hit.hit_point = origin + (dir_norm * sub_t);
                }
            }
        }
    }

    return closest_hit;
}
