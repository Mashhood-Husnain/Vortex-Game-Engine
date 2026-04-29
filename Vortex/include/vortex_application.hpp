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
#include <imgui.h>
#include <imgui_internal.h>
#include <stb_image.h>

#include "vortex_camera.hpp"
#include "vortex_shaders.hpp"
#include "vortex_shadowmanager.hpp"
#include "vortex_gui.hpp"
#include "vortex_postprocessor.hpp"
#include "vortex_skybox.hpp"
#include "vortex_particlesystem.hpp"
#include "vortex_model.hpp"
#include "vortex_grid.hpp"
#include "vortex_assetmanager.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_mouse.hpp"
#include "vortex_audio.hpp"
#include "vortex_objectmanager.hpp"
#include "vortex_uimanager.hpp"

#include "util/vortex_global_vars.hpp"
#include "util/vortex_gpu_pre_init.hpp"
#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"

class VortexSkybox;
class VortexGrid;

enum class EngineState
{
    EDITOR,
    PLAY
};

class VortexApplication
{
    GLFWwindow *window = nullptr;
    std::string window_name;
    bool is_fullscreen = false;
    int stored_window_x_pos;
    int stored_window_y_pos;
    int stored_window_width;
    int stored_window_height;
    float mouse_last_x = default_window_width / 2;
    float mouse_last_y = default_window_height / 2;
    bool first_mouse = true;
    float last_frame = 0.0f;

    bool enable_splash_screen = true; 
    bool is_playing_splash = true;
    GLuint splash_texture = 0;
    int splash_width = 0;
    int splash_height = 0;
    float splash_timer = 0.0f;

    VortexShader *worldaxis_shader = nullptr;
    unsigned int world_axisVAO;
    unsigned int world_axisVBO;

    VortexSkybox *skybox = nullptr;

    VortexGrid *environment_grid = nullptr;

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void check_key_press();
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    void change_window_size();
    void setup_world_axis_buffers();
    void draw_world_axis();
    void draw_world_axis_gizmo();
    static void window_close_callback(GLFWwindow* window);

    void load_splash_screen();
    void draw_splash_overlay(float dt);
public:
    int default_window_width;
    int default_window_height;

    EngineState current_state = EngineState::EDITOR;

    VortexCamera *camera = nullptr;
    VortexCamera *editor_camera = nullptr;

    bool show_exit_modal = false;
    bool has_unsaved_changes = false;

    float deltaTime = 0.0f;
    bool show_wireframe = false;
    bool show_mouse_cursor = true;
    bool view_world_axis = false;
    ShadowManager *shadow_manager = nullptr;
    VortexGUI gui;

    PostProcessor *post_processor = nullptr;

    VortexApplication(std::string window_name);
    ~VortexApplication();

    void run(std::function<void()> draw_callback);
    GLFWwindow* get_window_ptr();

    void set_post_processor(PostProcessor *post_processor);
    void set_skybox(VortexSkybox *skybox);

    void show_mouse(bool status);
    void request_exit();
};
