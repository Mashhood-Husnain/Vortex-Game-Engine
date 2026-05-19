#include "vortex_light.hpp"
#include "vortex_model.hpp"

void VortexLight::serialize(json &j)
{
    j["color"] = {color.x, color.y, color.z};
    j["intensity"] = intensity;
    j["ambient_strength"] = ambient_strength;
}

void VortexLight::deserialize(const json &j)
{
    if (j.contains("color")) color = glm::vec3(j["color"][0], j["color"][1], j["color"][2]);
    if (j.contains("intensity")) intensity = j["intensity"];
    if (j.contains("ambient_strength")) ambient_strength = j["ambient_strength"];
}

