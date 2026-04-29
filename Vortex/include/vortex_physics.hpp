#pragma once
#include "glm/glm.hpp"
#include "vortex_model.hpp"
#include "vortex_rigidbody.hpp"
#include "vortex_collider.hpp"

class VortexRigidbody;

struct OBB
{
    glm::vec3 center;
    glm::vec3 axes[3];
    glm::vec3 extents;
};

struct WorldAABB
{
    glm::vec3 min;
    glm::vec3 max;
};

struct CollisionHit
{
    bool has_hit = false;
    int hit_sub_object_index = -1;
};

struct RaycastHit
{
    bool has_hit = false;
    int hit_sub_object_index = -1;
    float distance = 0.0f;
    VortexModel* hit_model = nullptr;
    glm::vec3 hit_point = glm::vec3(0.0f);
};

struct Plane
{
    glm::vec3 normal = {0.0f, 1.0f, 0.0f};
    float distance = 0.0f;

    void normalize()
    {
        float mag = glm::length(normal);
        normal /= mag;
        distance /= mag;
    }
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;
};

class VortexPhysics
{
public:
    static bool check_collision(VortexModel *modela, VortexModel *modelb);
    static CollisionHit check_collision_detailed(VortexModel *modela, VortexModel *modelb);
    static RaycastHit raycast(glm::vec3 origin, glm::vec3 direction, float max_distance, std::vector<VortexModel*> ignore_list = {});
    static RaycastHit editor_raycast(glm::vec3 origin, glm::vec3 direction, float max_dist);
    static bool aabb_in_frustum(const glm::vec3 &min, const glm::vec3 &max, const Frustum &frustum);
};
