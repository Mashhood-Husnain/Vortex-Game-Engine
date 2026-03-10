/*
 * File: vortex_global_vars.hpp
 * Project: VortexEngine
 * Description: Global constant variables
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glm/glm.hpp>

namespace GLOBAL
{
    inline constexpr float GRAVITY = 9.80f;
    inline glm::vec3 LIGHTPOS = glm::vec3(0.0f, 10.0f, 5.0f);

    namespace COLORS
    {
        inline constexpr glm::vec4 SMOKE  = glm::vec4(0.7f, 0.7f, 0.7f, 0.4f);
        inline constexpr glm::vec4 BROWN  = glm::vec4(0.3f, 0.2f, 0.1f, 1.0f);
        inline constexpr glm::vec4 ORANGE = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
    }

    namespace DEFAULT_VERTICES
    {
        inline constexpr float PARTICLE_VERTICES[] = {
            -0.05f,  0.05f, 0.0f,
            -0.05f, -0.05f, 0.0f,
            0.05f, -0.05f, 0.0f,

            -0.05f,  0.05f, 0.0f,
            0.05f, -0.05f, 0.0f,
            0.05f,  0.05f, 0.0f
        };

        inline constexpr float WORLD_AXES_VERTICES[] = {
            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X-axis (Red)
            5.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // Y-axis (Green)
            0.0f, 5.0f, 0.0f,   0.0f, 1.0f, 0.0f,

            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // Z-axis (Blue)
            0.0f, 0.0f, 5.0f,   0.0f, 0.0f, 1.0f
        };
    }
}
