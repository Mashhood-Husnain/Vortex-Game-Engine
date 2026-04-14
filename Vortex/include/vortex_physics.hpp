#pragma once
#include "glm/glm.hpp"
#include "vortex_model.hpp"

struct WorldAABB {
    glm::vec3 min;
    glm::vec3 max;
};

class VortexPhysics
{
public:
    static bool check_collision(VortexModel *modela, VortexModel *modelb);
};
