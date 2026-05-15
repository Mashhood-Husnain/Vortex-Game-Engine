/*
 * File: vortex_camera.cpp
 * Project: VortexEngine
 * Description: Camera implementation
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_camera.hpp"
#include "vortex_application.hpp"

void VortexCamera::update_camera_vectors()
{
    glm::vec3 front_temp;
    front_temp.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front_temp.y = sin(glm::radians(pitch));
    front_temp.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(front_temp);

    right = glm::normalize(glm::cross(front, world_up));
    up = glm::normalize(glm::cross(right, front));
}

VortexCamera::VortexCamera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
{
    this->position = position;
    world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
    front = glm::vec3(0.0f, 0.0f, -1.0f);

    update_camera_vectors();
}

glm::mat4 VortexCamera::getViewMatrix()
{
    return glm::lookAt(position, position + front, up);
}

void VortexCamera::processKeyboard(const std::string& direction, float deltaTime)
{
    float velocity = movement_speed * deltaTime;

    if (direction == "FORWARD") position += front * velocity;
    if (direction == "BACKWARD") position -= front * velocity;
    if (direction == "LEFT") position -= right * velocity;
    if (direction == "RIGHT") position += right * velocity;
    if (direction == "UP") position += world_up * velocity;
    if (direction == "DOWN") position -= world_up * velocity;

}

void VortexCamera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    update_camera_vectors();
}

void VortexCamera::check_camera_movement(float deltaTime)
{
    if (VortexKeyboard::get_key("W")) processKeyboard("FORWARD", deltaTime);
    if (VortexKeyboard::get_key("S")) processKeyboard("BACKWARD", deltaTime);
    if (VortexKeyboard::get_key("A")) processKeyboard("LEFT", deltaTime);
    if (VortexKeyboard::get_key("D")) processKeyboard("RIGHT", deltaTime);
    if (VortexKeyboard::get_key("Q")) processKeyboard("UP", deltaTime);
    if (VortexKeyboard::get_key("E")) processKeyboard("DOWN", deltaTime);
}

glm::mat4 VortexCamera::getProjectionMatrix()
{
    return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);;
}

void VortexCamera::look_at(glm::vec3 target)
{
    glm::vec3 direction = glm::normalize(target - position);

    pitch = glm::degrees(asin(direction.y));
    yaw = glm::degrees(atan2(direction.z, direction.x));

    update_camera_vectors();
}

glm::vec3 VortexCamera::get_ray_from_mouse(glm::vec2 mouse_pos, VortexApplication *app)
{
    float x = (2.0f * mouse_pos.x) / app->get_width() - 1.0f;
    float y = 1.0f - (2.0f * mouse_pos.y) / app->get_height();
    
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0f, 1.0f);
    
    glm::mat4 invProjection = glm::inverse(getProjectionMatrix());
    glm::vec4 ray_eye = invProjection * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
    
    glm::mat4 invView = glm::inverse(getViewMatrix());
    glm::vec3 ray_world = glm::vec3(invView * ray_eye);
    
    return glm::normalize(ray_world);
}

Frustum VortexCamera::get_frustum()
{
    Frustum frustum;

    glm::mat4 view_proj = getProjectionMatrix() * getViewMatrix();

    frustum.leftFace.normal.x = view_proj[0][3] + view_proj[0][0];
    frustum.leftFace.normal.y = view_proj[1][3] + view_proj[1][0];
    frustum.leftFace.normal.z = view_proj[2][3] + view_proj[2][0];
    frustum.leftFace.distance = view_proj[3][3] + view_proj[3][0];
    frustum.leftFace.normalize();

    frustum.rightFace.normal.x = view_proj[0][3] - view_proj[0][0];
    frustum.rightFace.normal.y = view_proj[1][3] - view_proj[1][0];
    frustum.rightFace.normal.z = view_proj[2][3] - view_proj[2][0];
    frustum.rightFace.distance = view_proj[3][3] - view_proj[3][0];
    frustum.rightFace.normalize();

    frustum.bottomFace.normal.x = view_proj[0][3] + view_proj[0][1];
    frustum.bottomFace.normal.y = view_proj[1][3] + view_proj[1][1];
    frustum.bottomFace.normal.z = view_proj[2][3] + view_proj[2][1];
    frustum.bottomFace.distance = view_proj[3][3] + view_proj[3][1];
    frustum.bottomFace.normalize();

    frustum.topFace.normal.x = view_proj[0][3] - view_proj[0][1];
    frustum.topFace.normal.y = view_proj[1][3] - view_proj[1][1];
    frustum.topFace.normal.z = view_proj[2][3] - view_proj[2][1];
    frustum.topFace.distance = view_proj[3][3] - view_proj[3][1];
    frustum.topFace.normalize();

    frustum.nearFace.normal.x = view_proj[0][3] + view_proj[0][2];
    frustum.nearFace.normal.y = view_proj[1][3] + view_proj[1][2];
    frustum.nearFace.normal.z = view_proj[2][3] + view_proj[2][2];
    frustum.nearFace.distance = view_proj[3][3] + view_proj[3][2];
    frustum.nearFace.normalize();

    frustum.farFace.normal.x = view_proj[0][3] - view_proj[0][2];
    frustum.farFace.normal.y = view_proj[1][3] - view_proj[1][2];
    frustum.farFace.normal.z = view_proj[2][3] - view_proj[2][2];
    frustum.farFace.distance = view_proj[3][3] - view_proj[3][2];
    frustum.farFace.normalize();

    return frustum;
}

glm::vec3 VortexCamera::get_position() const { return position; }
glm::vec3 VortexCamera::get_front() const { return front; }
glm::vec3 VortexCamera::get_up() const { return up; }
glm::vec3 VortexCamera::get_right() const { return right; }

float VortexCamera::get_yaw() const { return yaw; }
float VortexCamera::get_pitch() const { return pitch; }
float VortexCamera::get_fov() const { return fov; }
float VortexCamera::get_aspect_ratio() const { return aspect_ratio; }
float VortexCamera::get_movement_speed() const { return movement_speed; }

void VortexCamera::set_position(const glm::vec3 &new_position) { position = new_position; }

void VortexCamera::set_movement_speed(float speed) { movement_speed = speed; };
void VortexCamera::set_mouse_sensitivity(float sensitivity) { movement_speed = sensitivity; }
void VortexCamera::set_aspect_ratio(float ratio) { aspect_ratio = ratio; }
void VortexCamera::set_fov(float new_fov) { fov = new_fov; }

void VortexCamera::set_rotation(float new_yaw, float new_pitch)
{
    yaw = new_yaw;
    pitch = new_pitch;

    update_camera_vectors();
}
