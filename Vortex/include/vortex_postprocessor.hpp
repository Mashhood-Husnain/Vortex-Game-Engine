/*
 * File: vortex_post_processor.hpp
 * Project: VortexEngine
 * Description: post processing of the screen
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glad/glad.h>
#include "vortex_shaders.hpp"

class PostProcessor
{
public:
    unsigned int fbo, texture, rbo;
    unsigned int quadVAO, quadVBO;
    VortexShader* screenShader;

    PostProcessor(std::string shader_vert_path, std::string shader_frag_path);

    void begin();
    void end();
    void draw(float time);
    void resize(int width, int height);

    ~PostProcessor();
};
