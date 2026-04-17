#pragma once
#include "glm/glm.hpp"
#include "vortex_model.hpp"

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

class VortexPhysics
{
public:
    static bool check_collision(VortexModel *modela, VortexModel *modelb);
    static CollisionHit check_collision_detailed(VortexModel *modela, VortexModel *modelb);
    static RaycastHit raycast(glm::vec3 origin, glm::vec3 direction, float max_distance);
};
