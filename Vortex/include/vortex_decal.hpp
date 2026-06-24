#pragma once

#include <string>
#include <glm/glm.hpp>
#include <json.hpp>

#include "vortex_assetmanager.hpp"
#include "vortex_model.hpp"
#include "vortex_behaviour.hpp"
#include "vortex_application.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_camera.hpp"

class VortexDecal : public VortexMonoBehaviour
{
public:
    VortexModel *quad_model = nullptr;
    std::string decal_texture_path = "";

    glm::vec3 position = glm::vec3(0.0f);
    glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(0.5f, 1.0f, 0.5f);

    bool flip_x = false;
    bool flip_y = false;

    void on_start() override;
    void draw(VortexCamera* camera, bool use_scene_lights);
    void set_texture(const std::string &path);

    void serialize(json &j) override;
    void deserialize(const json &j) override;

    ~VortexDecal();
};
