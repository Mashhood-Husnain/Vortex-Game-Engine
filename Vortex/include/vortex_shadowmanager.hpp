/*
 * File: vortex_shadowmanager.hpp
 * Project: VortexEngine
 * Description: Implementation of shadows in the world scene
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <functional>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "util/vortex_global_vars.hpp"

class ShadowManager
{
    void setup_shadow_map_buffers();
public:
    VortexShader *shadow_shader = nullptr;
    unsigned int shadow_map_FBO;
    unsigned int shadow_map;
    glm::mat4 light_space_matrix;
    bool is_active = false;

    ShadowManager();
    void draw_shadow_map(std::function<void()> draw_callback, VortexCamera *camera);
    ~ShadowManager();
};
