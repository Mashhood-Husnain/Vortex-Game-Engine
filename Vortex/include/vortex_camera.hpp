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
#include "vortex_global_vars.hpp"

class VortexApplication;

struct Frustum;

class VortexCamera
{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;

    float yaw;
    float pitch;

    float fov = 45.0f;
    float aspect_ratio;
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    float movement_speed = 10.0f;
    float mouse_sensitivity = 0.05f;

    void update_camera_vectors();
public:
    VortexCamera(glm::vec3 position=glm::vec3(0.0f, 5.0f, 8.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=-90.0f, float pitch=0.0f);

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    Frustum get_frustum();
    glm::vec3 get_ray_from_mouse(glm::vec2 mouse_pos, VortexApplication *app);

    void processKeyboard(const std::string& direction);
    void processMouseMovement(float xoffset, float yoffset);
    void check_camera_movement();
    void look_at(glm::vec3 target);

    glm::vec3 get_position() const;
    glm::vec3 get_front() const;
    glm::vec3 get_up() const;
    glm::vec3 get_right() const;

    float get_yaw() const;
    float get_pitch() const;
    float get_fov() const;
    float get_aspect_ratio() const;
    float get_movement_speed() const;

    void set_position(const glm::vec3 &new_position);

    void set_movement_speed(float speed);
    void set_mouse_sensitivity(float sensitivity);
    void set_aspect_ratio(float ratio);
    void set_fov(float new_fov);

    void set_rotation(float new_yaw, float new_pitch);
};
