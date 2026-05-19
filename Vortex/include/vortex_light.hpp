#pragma once

#include <glm/glm.hpp>
#include <json.hpp>

#include "vortex_behaviour.hpp"

class VortexModel;

class VortexLight : public VortexMonoBehaviour
{
public:
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    float ambient_strength = 0.1f;

    void serialize(json &j) override;
    void deserialize(const json &j) override;
};
