#pragma once

#include "vortex_model.hpp"
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_physics.hpp"
#include "vortex_assetmanager.hpp"

#include "util/vortex_global_vars.hpp"

#include <string>
#include <algorithm>

static GLuint occlusion_VAO = 0;
static GLuint occlusion_VBO = 0;

void setup_occlusion_cube();

class ParticleSystem;

class VortexObjectManager
{
    static VortexShader *model_shader;
    static VortexShader *collider_shader;
    static VortexShader *particle_shader;
    static VortexShader *unlit_shader;
    static VortexShader *depth_only_shader;
    static VortexShader *outline_shader;
    static VortexShader *decal_shader;
public:
    static std::vector<VortexModel*> active_models;
    static std::vector<VortexModel*> pending_models;
    static std::vector<ParticleSystem*> active_particlesystems;

    static void init();

    static void clean_up();
    static void init_scripts();
    static void update_scripts();
    static void clean_scripts();
    static void update_physics();
    static void update_particles();
    static void draw(VortexCamera &camera, bool show_wireframe, bool is_render_pass);
    static void check_object_status();

    static VortexShader *get_decal_shader();

    static void clear_scene();

    static VortexModel* get_object_by_tag(std::string tag);
    static void destroy_object(VortexModel *target_object);
};
