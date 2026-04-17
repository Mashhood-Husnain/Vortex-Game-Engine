/*
 * File: vortex_skybox.hpp
 * Project: VortexEngine
 * Description: Cubemap Skybox implementation
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glad/glad.h>
#include <vector>
#include <string>
#include <filesystem>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "util/vortex_global_vars.hpp"
#include "util/vortex_logs.hpp"

#include "stb_image.h"

#include <iostream>

class VortexSkybox
{
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    VortexShader *shader;

    unsigned int load_cubemap(std::vector<std::string> faces);
    void setup_mesh();

public:

    VortexSkybox(std::vector<std::string> faces);
    ~VortexSkybox();

    void draw(VortexCamera *camera);
};

std::vector<std::string> get_skyboxes(std::string path);
