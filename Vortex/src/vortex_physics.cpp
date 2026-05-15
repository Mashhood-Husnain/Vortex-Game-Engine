#include "vortex_physics.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

static OBB get_obb(const VortexBoxCollider& local_box, const glm::mat4& model_matrix)
{
    OBB obb;
    glm::vec3 local_center = (local_box.min + local_box.max) * 0.5f;
    
    obb.center = glm::vec3(model_matrix * glm::vec4(local_center, 1.0f));

    obb.axes[0] = glm::vec3(model_matrix[0]);
    obb.axes[1] = glm::vec3(model_matrix[1]);
    obb.axes[2] = glm::vec3(model_matrix[2]);

    obb.extents = (local_box.max - local_box.min) * 0.5f;
    obb.extents.x *= glm::length(obb.axes[0]);
    obb.extents.y *= glm::length(obb.axes[1]);
    obb.extents.z *= glm::length(obb.axes[2]);

    obb.axes[0] = glm::normalize(obb.axes[0]);
    obb.axes[1] = glm::normalize(obb.axes[1]);
    obb.axes[2] = glm::normalize(obb.axes[2]);

    return obb;
}

static bool test_OBB_overlap(const OBB& a, const OBB& b) 
{
    float R[3][3], AbsR[3][3];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R[i][j] = glm::dot(a.axes[i], b.axes[j]);
            AbsR[i][j] = std::abs(R[i][j]) + 1e-5f;
        }
    }

    glm::vec3 t = b.center - a.center;
    t = glm::vec3(glm::dot(t, a.axes[0]), glm::dot(t, a.axes[1]), glm::dot(t, a.axes[2]));

    float ra, rb;

    for (int i = 0; i < 3; i++) {
        ra = a.extents[i];
        rb = b.extents[0] * AbsR[i][0] + b.extents[1] * AbsR[i][1] + b.extents[2] * AbsR[i][2];
        if (std::abs(t[i]) > ra + rb) return false;
    }

    for (int i = 0; i < 3; i++) {
        ra = a.extents[0] * AbsR[0][i] + a.extents[1] * AbsR[1][i] + a.extents[2] * AbsR[2][i];
        rb = b.extents[i];
        if (std::abs(t[0] * R[0][i] + t[1] * R[1][i] + t[2] * R[2][i]) > ra + rb) return false;
    }

    ra = a.extents[1] * AbsR[2][0] + a.extents[2] * AbsR[1][0];
    rb = b.extents[1] * AbsR[0][2] + b.extents[2] * AbsR[0][1];
    if (std::abs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb) return false;

    ra = a.extents[1] * AbsR[2][1] + a.extents[2] * AbsR[1][1];
    rb = b.extents[0] * AbsR[0][2] + b.extents[2] * AbsR[0][0];
    if (std::abs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb) return false;

    ra = a.extents[1] * AbsR[2][2] + a.extents[2] * AbsR[1][2];
    rb = b.extents[0] * AbsR[0][1] + b.extents[1] * AbsR[0][0];
    if (std::abs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[2][0] + a.extents[2] * AbsR[0][0];
    rb = b.extents[1] * AbsR[1][2] + b.extents[2] * AbsR[1][1];
    if (std::abs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[2][1] + a.extents[2] * AbsR[0][1];
    rb = b.extents[0] * AbsR[1][2] + b.extents[2] * AbsR[1][0];
    if (std::abs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[2][2] + a.extents[2] * AbsR[0][2];
    rb = b.extents[0] * AbsR[1][1] + b.extents[1] * AbsR[1][0];
    if (std::abs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[1][0] + a.extents[1] * AbsR[0][0];
    rb = b.extents[1] * AbsR[2][2] + b.extents[2] * AbsR[2][1];
    if (std::abs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[1][1] + a.extents[1] * AbsR[0][1];
    rb = b.extents[0] * AbsR[2][2] + b.extents[2] * AbsR[2][0];
    if (std::abs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb) return false;

    ra = a.extents[0] * AbsR[1][2] + a.extents[1] * AbsR[0][2];
    rb = b.extents[0] * AbsR[2][1] + b.extents[1] * AbsR[2][0];
    if (std::abs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb) return false;

    return true;
}

static bool ray_intersect_local_box(glm::vec3 local_origin, glm::vec3 local_dir, const VortexBoxCollider& box, float& t_hit)
{
    glm::vec3 inv_dir = 1.0f / (local_dir + glm::vec3(1e-6f)); 

    glm::vec3 t0 = (box.min - local_origin) * inv_dir;
    glm::vec3 t1 = (box.max - local_origin) * inv_dir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tnear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tfar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (tnear > tfar || tfar < 0.0f) return false;

    t_hit = tnear;
    return true;
}

bool VortexPhysics::check_collision(VortexModel *modela, VortexModel *modelb)
{
    return check_collision_detailed(modela, modelb).has_hit;
}

CollisionHit VortexPhysics::check_collision_detailed(VortexModel *modela, VortexModel *modelb)
{
    CollisionHit result;
    if (!modela || !modelb || !modela->shared_data || !modelb->shared_data) return result;

    glm::mat4 matrix_a = modela->get_model_matrix();
    glm::mat4 matrix_b = modelb->get_model_matrix();

    OBB global_a = get_obb(modela->shared_data->collider, matrix_a);
    OBB global_b = get_obb(modelb->shared_data->collider, matrix_b);

    if (!test_OBB_overlap(global_a, global_b)) return result;

    bool has_inner_objects_a = modela->shared_data->objects.size() > 0;
    bool has_inner_objects_b = modelb->shared_data->objects.size() > 0;
    
    if (!has_inner_objects_a && !has_inner_objects_b)
    {
        result.has_hit = true;
        result.hit_sub_object_index = -1;
        return result;
    }
    
    std::vector<OBB> colliders_a;
    if (has_inner_objects_a)
    {
        for (size_t i = 0; i < modela->shared_data->objects.size(); i++)
        {
            if (!modela->active_parts[i]) continue;
            const auto& obj_a = modela->shared_data->objects[i];
            
            glm::mat4 sub_matrix_a = glm::translate(matrix_a, obj_a.transform.position);
            sub_matrix_a *= glm::mat4_cast(obj_a.transform.orientation);
            sub_matrix_a = glm::scale(sub_matrix_a, obj_a.transform.scale);

            colliders_a.push_back(get_obb(obj_a.collider, sub_matrix_a));
        }
    }
    else
    {
        colliders_a.push_back(global_a);
    }
    
    std::vector<OBB> colliders_b;
    if (has_inner_objects_b)
    {
        for (size_t j = 0; j < modelb->shared_data->objects.size(); j++)
        {
            if (!modelb->active_parts[j]) continue;
            const auto& obj_b = modelb->shared_data->objects[j];

            glm::mat4 sub_matrix_b = glm::translate(matrix_b, obj_b.transform.position);
            sub_matrix_b *= glm::mat4_cast(obj_b.transform.orientation);
            sub_matrix_b = glm::scale(sub_matrix_b, obj_b.transform.scale);

            colliders_b.push_back(get_obb(obj_b.collider, sub_matrix_b));
        }
    }
    else
    {
        colliders_b.push_back(global_b);
    }
    
    for (size_t i = 0; i < colliders_a.size(); i++)
    {
        for (size_t j = 0; j < colliders_b.size(); j++)
        {
            if (test_OBB_overlap(colliders_a[i], colliders_b[j]))
            {
                result.has_hit = true;
                result.hit_sub_object_index = has_inner_objects_b ? (int)j : -1;
                return result;
            }
        }
    }
    
    return result;
}

RaycastHit VortexPhysics::raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, std::vector<VortexModel*> ignore_list)
{
    RaycastHit closest_hit;
    closest_hit.distance = max_distance;

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        if (!model->is_active || model->should_destroy || !model->shared_data) continue;
        if (model->get_componant<VortexRigidbody>() == nullptr) continue;

        bool skip = false;
        for (VortexModel* ignored_model : ignore_list) {
            if (model == ignored_model) { skip = true; break; }
        }
        if (skip) continue;

        glm::mat4 matrix = model->get_model_matrix();
        glm::mat4 inv_matrix = glm::inverse(matrix);
        
        glm::vec3 local_origin = glm::vec3(inv_matrix * glm::vec4(origin, 1.0f));
        glm::vec3 local_dir = glm::normalize(glm::vec3(inv_matrix * glm::vec4(direction, 0.0f)));

        float global_t = 0.0f;
        if (!ray_intersect_local_box(local_origin, local_dir, model->shared_data->collider, global_t)) continue;

        glm::vec3 world_hit_approx = glm::vec3(matrix * glm::vec4(local_origin + local_dir * global_t, 1.0f));
        if (glm::length(world_hit_approx - origin) > closest_hit.distance) continue;

        for (size_t i = 0; i < model->shared_data->objects.size(); i++)
        {
            if (!model->active_parts[i]) continue;
            const auto& obj = model->shared_data->objects[i];

            glm::mat4 sub_matrix = glm::translate(matrix, obj.transform.position);
            sub_matrix *= glm::mat4_cast(obj.transform.orientation);
            sub_matrix = glm::scale(sub_matrix, obj.transform.scale);

            glm::mat4 inv_sub_matrix = glm::inverse(sub_matrix);
            glm::vec3 sub_local_origin = glm::vec3(inv_sub_matrix * glm::vec4(origin, 1.0f));
            glm::vec3 sub_local_dir = glm::normalize(glm::vec3(inv_sub_matrix * glm::vec4(direction, 0.0f)));

            float sub_t = 0.0f;
            if (ray_intersect_local_box(sub_local_origin, sub_local_dir, obj.collider, sub_t))
            {
                if (sub_t >= 0.0f)
                {
                    glm::vec3 sub_local_hit = sub_local_origin + sub_local_dir * sub_t;
                    glm::vec3 world_hit = glm::vec3(sub_matrix * glm::vec4(sub_local_hit, 1.0f));
                    float true_dist = glm::length(world_hit - origin);

                    if (true_dist < closest_hit.distance)
                    {
                        closest_hit.has_hit = true;
                        closest_hit.hit_model = model;
                        closest_hit.hit_sub_object_index = (int)i;
                        closest_hit.distance = true_dist;
                        closest_hit.hit_point = world_hit;
                    }
                }
            }
        }
    }

    return closest_hit;
}

RaycastHit VortexPhysics::editor_raycast(glm::vec3 origin, glm::vec3 direction, float max_dist)
{
    RaycastHit closest_hit;
    closest_hit.has_hit = false;
    float min_dist = max_dist;

    for (auto* model : VortexObjectManager::active_models)
    {
        if (!model->is_active || !model->shared_data) continue;

        glm::mat4 matrix = model->get_model_matrix();
        glm::mat4 inv_matrix = glm::inverse(matrix);
        
        glm::vec3 local_origin = glm::vec3(inv_matrix * glm::vec4(origin, 1.0f));
        glm::vec3 local_dir = glm::normalize(glm::vec3(inv_matrix * glm::vec4(direction, 0.0f)));

        float local_t;
        if (ray_intersect_local_box(local_origin, local_dir, model->shared_data->collider, local_t))
        {
            if (local_t >= 0.0f)
            {
                glm::vec3 local_hit = local_origin + local_dir * local_t;
                glm::vec3 world_hit = glm::vec3(matrix * glm::vec4(local_hit, 1.0f));
                float true_dist = glm::length(world_hit - origin);

                if (true_dist < min_dist)
                {
                    min_dist = true_dist;
                    
                    closest_hit.has_hit = true;
                    closest_hit.hit_model = model;
                    closest_hit.hit_point = world_hit;
                }
            }
        }
    }
    
    return closest_hit;
}

bool VortexPhysics::aabb_in_frustum(const glm::vec3 &min, const glm::vec3 &max, const Frustum &frustum)
{
    glm::vec3 center = (min + max) * 0.5f;
    glm::vec3 extents = max - center;

    const Plane *planes[6] = {
        &frustum.leftFace, &frustum.rightFace,
        &frustum.bottomFace, &frustum.topFace,
        &frustum.nearFace, &frustum.farFace
    };

    for (int i = 0; i < 6; i++)
    {
        float radius = extents.x * std::abs(planes[i]->normal.x) +
                       extents.y * std::abs(planes[i]->normal.y) +
                       extents.z * std::abs(planes[i]->normal.z);
        
        float distance = glm::dot(center, planes[i]->normal) + planes[i]->distance;

        if (distance < -radius) return false;
    }

    return true;
}
