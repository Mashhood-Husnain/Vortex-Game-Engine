/*
 * File: vortex_window.hpp
 * Project: VortexEngine
 * Description: Implementation of window
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <functional>
#include <algorithm>

#include "vortex_camera.hpp"
#include "vortex_shaders.hpp"
#include "vortex_shadowmanager.hpp"
#include "vortex_gui.hpp"
#include "vortex_postprocessor.hpp"
#include "vortex_skybox.hpp"
#include "vortex_particlesystem.hpp"
#include "vortex_model.hpp"
#include "vortex_grid.hpp"

#include "util/vortex_global_vars.hpp"
#include "util/vortex_gpu_pre_init.hpp"

class VortexApplication
{
    GLFWwindow *window = nullptr;
    int default_window_width;
    int default_window_height;
    std::string window_name;
    bool is_fullscreen = false;
    int stored_window_x_pos;
    int stored_window_y_pos;
    int stored_window_width;
    int stored_window_height;
    VortexCamera *camera = nullptr;
    float mouse_last_x = default_window_width / 2;
    float mouse_last_y = default_window_height / 2;
    bool first_mouse = true;
    float last_frame = 0.0f;

    VortexShader *worldaxis_shader = nullptr;
    unsigned int world_axisVAO;
    unsigned int world_axisVBO;
    bool view_world_axis = false;

    PostProcessor *post_processor = nullptr;
    VortexSkybox *skybox = nullptr;

    std::vector<ParticleSystem*> dynamic_particlesystems;
    VortexShader *particle_shader = nullptr;

    std::vector<VortexModel*> dynamic_models;
    VortexShader *model_shader = nullptr;

    VortexGrid *environment_grid = nullptr;

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void change_window_size();
    void setup_world_axis_buffers();
    void draw_world_axis();
    void draw_world_axis_gizmo();
public:
    float deltaTime = 0.0f;
    bool show_wireframe = false;
    bool show_mouse_cursor = false;
    ShadowManager *shadow_manager = nullptr;
    VortexGUI gui;

    VortexApplication(std::string window_name, int width, int height);
    ~VortexApplication();

    void run(std::function<void()> draw_callback);
    GLFWwindow* get_window_ptr();

    void set_post_processor(PostProcessor *post_processor);
    void set_skybox(VortexSkybox *skybox);
};
