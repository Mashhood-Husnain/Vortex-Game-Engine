/*
 * File: vortex_camera.hpp
 * Project: VortexEngine
 * Description: Camera implementation
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

#include "vortex_keyboard.hpp"
#include "vortex_physics.hpp"

class VortexApplication;

struct Frustum;

class VortexCamera
{
    float near_plane = 0.1f;
    float far_plane = 1000.0f;
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;

    float yaw, pitch;
    float fov = 45.0f;

    float movement_speed = 10.0f;
    float mouse_sensitivity = 0.05f;

    float aspect_ratio;
    bool anchored;

    VortexCamera(glm::vec3 position=glm::vec3(0.0f, 5.0f, 8.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=-90.0f, float pitch=0.0f);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    glm::vec3 get_ray_from_mouse(glm::vec2 mouse_pos, VortexApplication *app);

    void processKeyboard(const std::string& direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);

    void look_at(glm::vec3 target);

    void update_camera_vectors();
    void check_camera_movement(float deltaTime);

    Frustum get_frustum();
};
