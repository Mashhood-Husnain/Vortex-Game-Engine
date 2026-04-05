/*
 * File: vortex_player.cpp
 * Project: VortexEngine
 * Description: Implementation of player
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_player.hpp"

Player::Player(const std::string& name, VortexCamera* camera, VortexModel* player_body, VortexShader* shader, glm::vec3 starting_position)
{
    player_name = name;
    if (camera)
    {
        player_camera = camera;
        player_camera->anchored = true;
    }
    player_position = starting_position;
    this->player_body = player_body;
    player_shader = shader;
    player_height = player_body->model_height;
    
    if (camera)
    {
        update_player_vectors();
    }
}

void Player::update_player_vectors()
{
    glm::vec3 new_front;
    new_front.x = cos(glm::radians(player_camera->yaw));
    new_front.y = 0.0f;
    new_front.z = sin(glm::radians(player_camera->yaw));
    front = glm::normalize(new_front);

    right = glm::normalize(glm::cross(front, world_up));
}

void Player::processKeyboard(const std::string& direction, float deltaTime)
{
    update_player_vectors();

    float velocity = movement_speed * deltaTime;

    if (direction == "FORWARD")  player_position += front * velocity;
    if (direction == "BACKWARD") player_position -= front * velocity;
    if (direction == "LEFT")     player_position -= right * velocity;
    if (direction == "RIGHT")    player_position += right * velocity;
    
    player_camera->position = player_position + player_head_offset;
}

void Player::update(VortexApplication *window)
{
    if (!player_body) return;

    player_body->position = player_position;
    if (player_camera)
    {
        // apply gravity for when jump mechanics get written

        player_head_offset = glm::vec3(0.0f, player_height * player_body->scale.y * 0.95f, 0.0f);
        if (!window->show_mouse_cursor)
        {
            check_player_movement(window->get_window_ptr(), window->deltaTime);
        }

        float distance_moved = glm::length(player_position - last_position);
        last_position = player_position;

        float current_speed = 0.0f;
        if (window->deltaTime > 0.0f) 
        {
            current_speed = distance_moved / window->deltaTime;
        }

        float max_bob_amount = 0.08f; // Max camera height shift at full speed
        float base_bob_speed = 15.0f; // Base frequency for the sine wave

        float speed_ratio = glm::clamp(current_speed / movement_speed, 0.0f, 1.5f);
        float target_bob_amount = max_bob_amount * speed_ratio;
        current_bob_amount = glm::mix(current_bob_amount, target_bob_amount, window->deltaTime * 10.0f);

        if (current_bob_amount > 0.001f) 
        {
            float dynamic_speed = base_bob_speed * glm::max(speed_ratio, 0.5f);
            bob_timer += window->deltaTime * dynamic_speed;
        }
        else
        {
            bob_timer = 0.0f; 
        }

        float wobble_y = sin(bob_timer) * current_bob_amount;
        float wobble_x = cos(bob_timer * 0.5f) * current_bob_amount;

        if (player_camera->anchored)
        {
            player_camera->position = player_position + player_head_offset + glm::vec3(wobble_x, wobble_y, 0.0f);
            player_body->rotation.y = 90.0f - player_camera->yaw;
            player_body->draw(*window->shadow_manager->shadow_shader, *player_camera, false);
        }
        else
        {
            player_body->draw(*player_shader, *player_camera, false);
        }
    }
}

Player::~Player()
{
    player_camera = nullptr;
    player_body = nullptr;
    player_shader = nullptr;
}

void Player::check_player_movement(GLFWwindow* window, float deltaTime)
{
    if (!player_camera->anchored) return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) processKeyboard("FORWARD", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) processKeyboard("BACKWARD", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) processKeyboard("LEFT", deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) processKeyboard("RIGHT", deltaTime);
}
