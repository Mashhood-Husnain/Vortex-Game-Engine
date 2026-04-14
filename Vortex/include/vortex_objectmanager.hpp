#pragma once

#include "vortex_model.hpp"
#include "vortex_particlesystem.hpp"
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"

#include <string>

class VortexObjectManager
{
    static VortexShader *model_shader;
    static VortexShader *collider_shader;
    static VortexShader *particle_shader;
public:
    static std::vector<VortexModel*> active_models;
    static std::vector<VortexModel*> pending_models;
    static std::vector<ParticleSystem*> active_particlesystems;

    static void init();

    static void clean_up();
    static void update(float deltaTime);
    static void draw(VortexCamera &camera, bool show_wireframe);
    static void check_object_status();

    static VortexModel* get_object_by_tag(std::string tag);
};
