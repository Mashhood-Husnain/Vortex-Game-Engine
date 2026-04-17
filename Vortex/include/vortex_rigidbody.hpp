#pragma once

#include "vortex_behaviour.hpp"
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"
#include "util/vortex_script_registry.hpp"

#include <glm/glm.hpp>

class VortexRigidbody : public VortexMonoBehaviour
{
    bool check_world_collision();
public:
    glm::vec3 velocity = glm::vec3(0.0f);
    float gravity = 25.0f;
    
    bool is_kinematic = false;
    bool is_grounded = false;

    void on_update(float deltaTime) override;
};
