#include "vortex_physics.hpp"

static bool test_AABB_overlap(const WorldAABB& a, const WorldAABB& b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool VortexPhysics::check_collision(VortexModel *modela, VortexModel *modelb)
{
    if (!modela || !modelb) return false; 
    if (!modela->shared_data || !modelb->shared_data) return false;

    glm::vec3 scale_a = modela->transform.scale * modela->collider_scale;
    glm::vec3 scale_b = modelb->transform.scale * modelb->collider_scale;
    float padding = 0.05f;

    WorldAABB global_box_a, global_box_b;

    global_box_a.min = modela->transform.position + (modela->shared_data->collider.min * scale_a) - padding;
    global_box_a.max = modela->transform.position + (modela->shared_data->collider.max * scale_a) + padding;

    global_box_b.min = modelb->transform.position + (modelb->shared_data->collider.min * scale_b) - padding;
    global_box_b.max = modelb->transform.position + (modelb->shared_data->collider.max * scale_b) + padding;

    if (!test_AABB_overlap(global_box_a, global_box_b)) 
    {
        return false;
    }
    
    for (const auto& obj_a : modela->shared_data->objects)
    {
        WorldAABB sub_box_a;
        sub_box_a.min = modela->transform.position + (obj_a.collider.min * scale_a) - padding;
        sub_box_a.max = modela->transform.position + (obj_a.collider.max * scale_a) + padding;

        for (const auto& obj_b : modelb->shared_data->objects)
        {
            WorldAABB sub_box_b;
            sub_box_b.min = modelb->transform.position + (obj_b.collider.min * scale_b) - padding;
            sub_box_b.max = modelb->transform.position + (obj_b.collider.max * scale_b) + padding;

            if (test_AABB_overlap(sub_box_a, sub_box_b))
            {
                return true;
            }
        }
    }

    return false;
}
