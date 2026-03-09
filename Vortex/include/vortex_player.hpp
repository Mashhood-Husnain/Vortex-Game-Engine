/*
 * File: vortex_player.hpp
 * Project: VortexEngine
 * Description: Implementation of player
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <iostream>
#include <string>

// #include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vortex_camera.hpp"
#include "vortex_objreader.hpp"
#include "vortex_shaders.hpp"

class Player
{
    std::string player_name;
    VortexCamera* player_camera = nullptr;
    float movement_speed = 5.0f;
    glm::vec3 player_head_offset;
    VortexShader *player_shader = nullptr;
    float player_height;

    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);

    void update_player_vectors();
    void check_player_movement(GLFWwindow* window, float deltaTime);
    void processKeyboard(const std::string& direction, float deltaTime);

public:
    VortexModel *player_body = nullptr;
    glm::vec3 player_position;
    Player(const std::string& name, VortexCamera* camera, VortexModel* player_body, VortexShader* shader, glm::vec3 starting_position=glm::vec3(0.0f, 0.0f, 10.0f));
    void update(GLFWwindow* window, float deltaTime);
    ~Player();
};
