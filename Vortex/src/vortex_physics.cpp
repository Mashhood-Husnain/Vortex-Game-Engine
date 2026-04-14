#include "vortex_physics.hpp"

bool VortexPhysics::check_collision(VortexModel *modela, VortexModel *modelb)
{
    if (!modela || !modelb) return false; 

    WorldAABB box_a, box_b;
    
    glm::vec3 scale_a = modela->transform.scale * modela->collider_scale;
    glm::vec3 scale_b = modelb->transform.scale * modelb->collider_scale;

    float padding = 0.05f;

    box_a.min = modela->transform.position + (modela->shared_data->collider.min * scale_a) - padding;
    box_a.max = modela->transform.position + (modela->shared_data->collider.max * scale_a) + padding;

    box_b.min = modelb->transform.position + (modelb->shared_data->collider.min * scale_b) - padding;
    box_b.max = modelb->transform.position + (modelb->shared_data->collider.max * scale_b) + padding;

    return (box_a.min.x <= box_b.max.x && box_a.max.x >= box_b.min.x) &&
           (box_a.min.y <= box_b.max.y && box_a.max.y >= box_b.min.y) &&
           (box_a.min.z <= box_b.max.z && box_a.max.z >= box_b.min.z);
}
