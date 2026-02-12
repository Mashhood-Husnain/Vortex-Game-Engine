#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>

class OpenGLCamera
{
    void update_camera_vectors();
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;

    float yaw, pitch;

    float movement_speed = 15.0f;
    float mouse_sensitivity = 0.1f;

    float aspect_ratio;

    OpenGLCamera(glm::vec3 position=glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up=glm::vec3(0.0f, 1.0f, 0.0f), float yaw=-90.0f, float pitch=0.0f);
    glm::mat4 getViewMatrix();
    void processKeyboard(const std::string& direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
};
