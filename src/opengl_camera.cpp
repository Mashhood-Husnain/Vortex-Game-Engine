#include "opengl_camera.hpp"

void OpenGLCamera::update_camera_vectors()
{
    glm::vec3 front_temp;
    front_temp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front_temp.y = sin(glm::radians(pitch));
    front_temp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front_temp);

    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}

OpenGLCamera::OpenGLCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
        : position(position), world_up(up), yaw(yaw), pitch(pitch), front(glm::vec3(0.0f, 0.0f, -1.0f))
{
        update_camera_vectors();
}

glm::mat4 OpenGLCamera::getViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

void OpenGLCamera::processKeyboard(const std::string& direction, float deltaTime)
{
    float velocity = movement_speed * deltaTime;

    if (direction == "FORWARD") position += front * velocity;
    if (direction == "BACKWARD") position -= front * velocity;
    if (direction == "LEFT") position -= right * velocity;
    if (direction == "RIGHT") position += right * velocity;
    if (direction == "UP") position += world_up * velocity;
    if (direction == "DOWN") position -= world_up * velocity;

}

void OpenGLCamera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    update_camera_vectors();
}
