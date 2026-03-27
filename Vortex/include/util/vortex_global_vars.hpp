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
    inline constexpr float GRAVITY = 9.80f;
    inline glm::vec3 LIGHTPOS = glm::vec3(0.0f, 10.0f, 5.0f);
    inline const std::string VORTEX_VERSION = "\"0.1.0\"";

    inline std::vector<std::string> SKYBOX_FACES_PATH = {
        // Order: right, left top, bottom, front, back
        "assets/images/skybox/space_right.png",
        "assets/images/skybox/space_left.png",
        "assets/images/skybox/space_top.png",
        "assets/images/skybox/space_bottom.png",
        "assets/images/skybox/space_front.png",
        "assets/images/skybox/space_back.png"
    };

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
    }
}
