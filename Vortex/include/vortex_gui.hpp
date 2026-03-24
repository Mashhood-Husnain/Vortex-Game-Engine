#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <set>
#include <filesystem>
#include <vector>

#include "util/vortex_engine_stats.hpp"

class VortexModel;
class VortexCamera;
class VortexWindow;

class VortexGUI
{
    std::set<VortexModel*> m_processed_models;
    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_display_names;
    int m_selected_shader_idx = 0;
    bool m_shaders_loaded = false;

    void refresh_shader_list();
public:
    VortexGUI();
    ~VortexGUI();

    void init(GLFWwindow* window);
    void update();
    void render();
    void begin_scene_inspector();
    void end_scene_inspector();
    void show_engine_stats();
    void show_model_info(VortexModel *model);
    void show_camera_info(VortexCamera *camera);
    void show_post_process_options(VortexWindow *window);
};
