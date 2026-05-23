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
#include "vortex_debugrenderer.hpp"

#include "util/vortex_global_vars.hpp"
#include "util/vortex_gpu_pre_init.hpp"
#include "util/vortex_save_load.hpp"
#include "util/vortex_logs.hpp"
#include "util/vortex_game_api.hpp"
#include "util/vortex_game_reloader.hpp"
#include "util/vortex_script_registry.hpp"

class VortexSkybox;
class VortexGrid;

enum class EngineState
{
    EDITOR,
    PLAY,
    PROJECT_HUB
};

class VortexApplication
{
private:
    GLFWwindow *window = nullptr;
    std::string window_name;

    int default_window_width;
    int default_window_height;
    bool is_fullscreen = false;

    int stored_window_x_pos  = 0.0f;
    int stored_window_y_pos  = 0.0f;
    int stored_window_width  = 0.0f;
    int stored_window_height = 0.0f;

    float mouse_last_x = 0.0f;
    float mouse_last_y = 0.0f;
    bool first_mouse = true;
    float last_frame = 0.0f;
    float deltaTime = 0.0f;

    bool enable_splash_screen = true; 
    bool is_playing_splash = true;
    GLuint splash_texture = 0;
    int splash_width = 0;
    int splash_height = 0;
    float splash_timer = 0.0f;

    EngineState current_state = EngineState::PROJECT_HUB;
    bool show_exit_modal = false;
    bool has_unsaved_changes = false;
    bool show_wireframe = false;
    bool show_mouse_cursor = true;

    VortexCamera *camera = nullptr;
    VortexCamera *editor_camera = nullptr;
    PostProcessor *post_processor = nullptr;
    VortexSkybox *skybox = nullptr;
    VortexGrid *environment_grid = nullptr;
    ShadowManager *shadow_manager = nullptr;

    VortexShader *worldaxis_shader = nullptr;
    unsigned int world_axisVAO;
    unsigned int world_axisVBO;

    GameMemory game_memory;
    GameCode *game_code = nullptr;

    GLFWmonitor* get_current_monitor(GLFWwindow* window);
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
    static void window_close_callback(GLFWwindow* window);
    void check_key_press();
    void change_window_size();
    void setup_world_axis_buffers();
    void load_splash_screen();
    void draw_splash_overlay(float dt);
    void init_game_code();
    void check_for_hot_reload();
public:
    VortexApplication(std::string window_name);
    ~VortexApplication();

    void run(std::function<void()> draw_callback);

    void set_post_processor(PostProcessor *post_processor);
    void set_skybox(VortexSkybox *skybox);
    void show_mouse(bool status);
    void request_exit();

    void enter_play_mode();
    void trigger_compile();

    GLFWwindow* get_window_ptr() const;
    float get_delta_time() const;
    int get_width() const;
    int get_height() const;

    EngineState get_state() const;
    bool is_wireframe_enabled() const;
    bool is_exit_modal_open() const;
    bool has_unsaved() const;

    VortexCamera *get_camera() const;
    VortexCamera *get_editor_camera() const;
    ShadowManager *get_shadow_manager() const;

    void set_camera(VortexCamera *camera);

    void set_state(EngineState state);
    void toggle_wireframe(bool enable);
    void toggle_exit_modal(bool show);

    void mark_unsaved_changes();
    void clear_unsaved_changes();
};
