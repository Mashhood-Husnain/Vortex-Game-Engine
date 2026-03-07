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

void Player::update(GLFWwindow* window, float deltaTime)
{
    if (!player_body) return;

    player_body->position = player_position;
    if (player_camera)
    {
        // apply gravity for when jump mechanics get written

        player_head_offset = glm::vec3(0.0f, player_height * player_body->scale.y * 0.95f, 0.0f);
        check_player_movement(window, deltaTime);

        if (player_camera->anchored)
        {
            player_camera->position = player_position + player_head_offset;
            player_body->rotation.y = 90.0f - player_camera->yaw;
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
