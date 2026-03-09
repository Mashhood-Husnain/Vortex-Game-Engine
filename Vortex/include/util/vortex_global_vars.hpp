/*
 * File: vortex_global_vars.hpp
 * Project: VortexEngine
 * Description: Global constant variables
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glm/glm.hpp>

#define global_WORLDGRAVITY 9.80f

namespace COLORS {
    inline const glm::vec4 SMOKE  = glm::vec4(0.7f, 0.7f, 0.7f, 0.4f);
    inline const glm::vec4 BROWN  = glm::vec4(0.3f, 0.2f, 0.1f, 1.0f);
    inline const glm::vec4 ORANGE = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
}

static const float global_DEFAULTPARTICLEVERTICES[] = {
    -0.05f,  0.05f, 0.0f,
    -0.05f, -0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,

    -0.05f,  0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,
     0.05f,  0.05f, 0.0f
};

static const float global_WORLDAXESVERTICES[] = {
    0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X-axis (Red)
    5.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // Y-axis (Green)
    0.0f, 5.0f, 0.0f,   0.0f, 1.0f, 0.0f,

    0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // Z-axis (Blue)
    0.0f, 0.0f, 5.0f,   0.0f, 0.0f, 1.0f
};
