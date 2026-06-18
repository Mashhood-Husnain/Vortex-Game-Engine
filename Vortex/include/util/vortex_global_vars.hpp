/*
 * File: vortex_global_vars.hpp
 * Project: VortexEngine
 * Description: Global constant variables
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glm/glm.hpp>
#include <string>

namespace GLOBAL
{
    inline float deltaTime = 0.0f;
    inline constexpr float GRAVITY = 9.80f;
    inline glm::vec3 LIGHTPOS = glm::vec3(0.0f, 10.0f, 5.0f);
    inline const std::string VORTEX_VERSION = "\"0.1.0\"";

    namespace DEFAULT_VERTICES
    {
        inline constexpr float PARTICLE_VERTICES[] = {
            // X,    Y,    Z,      U,    V
            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // Top-left
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // Bottom-left
             0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // Bottom-right

            -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // Top-left
             0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // Bottom-right
             0.5f,  0.5f, 0.0f,   1.0f, 1.0f   // Top-right
        };

        inline constexpr float WORLD_AXES_VERTICES[] = {
            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X-axis (Red)
            5.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // Y-axis (Green)
            0.0f, 5.0f, 0.0f,   0.0f, 1.0f, 0.0f,

            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // Z-axis (Blue)
            0.0f, 0.0f, 5.0f,   0.0f, 0.0f, 1.0f
        };

        inline constexpr float SKYBOX_VERTICES[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };

        inline constexpr float OCCLUSION_VERTICES[] = {
            // Front face
            -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,
            // Back face
            -0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
            // Left face
            -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
            // Right face
            0.5f,  0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,
            // Top face
            -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
            // Bottom face
            -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f
        };
    }
}
