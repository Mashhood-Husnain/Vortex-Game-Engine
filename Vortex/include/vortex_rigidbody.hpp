#pragma once

#include "vortex_behaviour.hpp"
#include <json.hpp>
#include "vortex_physics.hpp"
#include "vortex_objectmanager.hpp"

#include <glm/glm.hpp>

class VortexRigidbody : public VortexMonoBehaviour
{
    bool check_world_collision();
public:
    glm::vec3 velocity = glm::vec3(0.0f);
    float gravity_value = 25.0f;

    bool is_kinematic = false;
    bool is_grounded = false;
    bool gravity = false;

    void on_update() override;

    void serialize(json &j) override;
    void deserialize(const json &j) override;
};
