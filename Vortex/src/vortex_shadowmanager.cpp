/*
 * File: vortex_shadowmanager.cpp
 * Project: VortexEngine
 * Description: Implementation of shadows in the world scene
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_shadowmanager.hpp"

ShadowManager::ShadowManager()
{
    shadow_shader = new VortexShader("shaders/shadowmap.vert", "shaders/shadowmap.frag");
    setup_shadow_map_buffers();    
}

void ShadowManager::setup_shadow_map_buffers()
{
    glGenFramebuffers(1, &shadow_map_FBO);

    glGenTextures(1, &shadow_map);
    glBindTexture(GL_TEXTURE_2D, shadow_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowManager::draw_shadow_map(std::function<void()> draw_callback, VortexCamera *camera)
{
    is_active = true;

    float near_plane = 5.0f;
    float far_plane = 70.0f;
    float size = 15.0f;

    float worldUnitsPerTexel = (size * 2.0f) / 4096.0f;
    glm::vec3 position;
    position.x = floor(camera->position.x / worldUnitsPerTexel) * worldUnitsPerTexel;
    position.y = floor(camera->position.y / worldUnitsPerTexel) * worldUnitsPerTexel;
    position.z = floor(camera->position.z / worldUnitsPerTexel) * worldUnitsPerTexel;

    glm::mat4 light_projection = glm::ortho(-size, size, -size, size, near_plane, far_plane);

    glm::vec3 lightDirection = glm::normalize(GLOBAL::LIGHTPOS);
    float distance = 50.0f;
    glm::vec3 lightCameraPos = position + (lightDirection * distance);

    glm::mat4 light_view = glm::lookAt(lightCameraPos, position, glm::vec3(0.0f, 1.0f, 0.0f));
    light_space_matrix = light_projection * light_view;

    glViewport(0, 0, 4096, 4096);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    draw_callback();
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    is_active = false;
}

ShadowManager::~ShadowManager()
{
    delete shadow_shader;
    shadow_shader = nullptr;
}
