#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class VortexCamera
{
    void update_camera_vectors();

    float fov = 45.0f;
    float near_plane = 0.1f;
    float far_plane = 100.0f;
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;

    float yaw, pitch;

    float movement_speed = 5.0f;
    float mouse_sensitivity = 0.05f;

    float aspect_ratio;
    bool anchored;

    VortexCamera(glm::vec3 position=glm::vec3(0.0f, 5.0f, 8.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=-90.0f, float pitch=0.0f);
    glm::mat4 getViewMatrix();
    void processKeyboard(const std::string& direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void check_camera_movement(GLFWwindow* window, float deltaTime);
    glm::mat4 getProjectionMatrix();
    void look_at(glm::vec3 target);
};
