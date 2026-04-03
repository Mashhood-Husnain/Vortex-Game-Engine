#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <set>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "util/vortex_engine_stats.hpp"

class VortexModel;
class VortexCamera;
class VortexWindow;
class ParticleSystem;

namespace VortexGuiLambda
{
    inline auto ClampedInputFloat = [](const char* label, float *v, float step, float step_fast, float v_min, float v_max, const char *format="%.2f")
    {
        if (ImGui::InputFloat(label, v, step, step_fast, format))
        {
            *v = std::clamp(*v, v_min, v_max);
        }
    };

    inline auto ClampedInputInt = [](const char *label, int *v, int step, int step_fast, int v_min, int v_max)
    {
        if (ImGui::InputInt(label, v, step, step_fast))
        {
            *v = std::clamp(*v, v_min, v_max);
        }
    };

    inline auto DataGetter = [](void* data, int idx) -> const char*
    {
        auto* items = static_cast<std::vector<std::string>*>(data);

        if (idx < 0 || idx >= static_cast<int>(items->size()))
        {
            return nullptr;
        }

        return (*items)[idx].c_str();
    };
}

class VortexGUI
{
    std::set<VortexModel*> m_processed_models;

    std::set<ParticleSystem*> m_processed_ps;

    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_display_names;
    int m_selected_shader_idx = 0;
    bool m_shaders_loaded = false;

    std::vector<std::vector<std::string>> m_skybox_files;
    std::vector<std::string> m_skybox_display_names;
    int m_selected_skybox_idx = 0;
    bool m_skybox_loaded = false;

    void refresh_shader_list();
    void refresh_skybox_list();
public:
    VortexGUI();
    ~VortexGUI();

    void init(GLFWwindow* window);
    void update();
    void render();

    void begin_scene_inspector();
    void show_inspector_info(VortexModel *model, ParticleSystem *ps);
    void end_scene_inspector();

    void show_camera_info(VortexCamera *camera);

    void show_post_process_options(VortexWindow *window);
    void show_skybox_options(VortexWindow *window);

    void show_engine_stats();
};
