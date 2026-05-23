#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <set>
#include <map>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <ImGuizmo.hpp>
#include <mutex>
#include <atomic>
#include <cstdio>

#include "util/vortex_engine_stats.hpp"
#include "util/vortex_logs.hpp"

class VortexApplication;
class VortexModel;
class VortexCamera;
class ParticleSystem;
class VortexProject;
class ScriptRegistry;
class VortexMonoBehaviour;
class VortexEditor;

enum class SnapshotState;

struct CompilerState
{
    inline static std::atomic<bool> is_compiling{false};
    inline static std::atomic<float> progress{0.0f};
    inline static std::mutex status_mutex;
    inline static std::string status_text = "Initializing compiler...";
};

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

struct HUDElement
{
    std::string label;
    float* value_ptr;
    float max_value;
    ImVec4 color;
    bool is_visible = true;
};

struct FolderNode
{
    std::map<std::string, FolderNode> subfolders;
    std::vector<VortexModel*> models;
};

class VortexGUI
{
    static VortexApplication *app;

    static std::set<VortexModel*> m_processed_models;

    static std::set<ParticleSystem*> m_processed_ps;

    static std::vector<std::string> m_shader_files;
    static std::vector<std::string> m_display_names;
    static bool m_shaders_loaded;

    static std::vector<std::vector<std::string>> m_skybox_files;
    static std::vector<std::string> m_skybox_display_names;
    static bool m_skybox_loaded;

    ImVec2 m_scene_pos = ImVec2(0, 0);
    ImVec2 m_scene_size = ImVec2(0, 0);

    static std::vector<std::string> m_available_model_names;
    static std::vector<std::string> m_available_model_paths;
    static bool m_models_scanned;

    static unsigned int scene_fbo;
    static unsigned int scene_rbo;
    static unsigned int scene_texture;

    static char m_project_to_delete[128];

    static void refresh_skybox_list();
    static void gui_set_skybox();

    static void refresh_shader_list();
    static void gui_set_post_processor();

    static void handle_picking(VortexCamera *camera, ImVec2 image_pos);
    static void handle_shortcuts();
    static void snap_to_floor();

    static void setup_scene_fbo(int width, int height);
public:
    static std::set<VortexModel*> m_selected_models;
    static ImGuizmo::OPERATION m_current_op;
    static bool m_is_using_gizmo;

    static std::vector<std::string> explicit_empty_folders;

    static int scene_width;
    static int scene_height;

    static int viewport_width;
    static int viewport_height;

    static bool show_inspector;
    static bool show_camera_info;
    static bool show_creator_window;
    static bool show_engine_stats;
    static bool show_terminal;
    static bool show_skybox_post_process_options;    

    static int m_selected_skybox_idx;
    static int m_selected_shader_idx;

    static char m_new_project_name[128];

    static std::string _vendor_;
    static std::string _renderer_;

    VortexGUI();

    static void build_dockspace();
    static void draw_editor_viewport(float deltaTime, VortexCamera* camera);
    static void resize_scene_fbo(int width, int height);
    static void bind_framebuffer();

    static void init(VortexApplication* app, int width, int height);
    static void update();
    static void render();

    static void scene_inspector();
    static void inspect_model(VortexModel *model);
    static void inspect_particle_system(ParticleSystem *ps);
    static void draw_model_info_panel(VortexModel* model, int flags);
    static void draw_model_components_panel(VortexModel* model, int flags);
    static void draw_model_transform_panel(VortexModel* model, int flags);
    static void draw_model_actions(VortexModel* model);

    static void draw_main_menu_bar();
    static void draw_project_hub();

    static void creator_window();
    static void camera_info(VortexCamera *camera);

    static void post_process_options();
    static void skybox_options();

    static void draw_exit_modal();

    static void engine_stats();

    static void draw_terminal();
    static void draw_compiler_modal();

    static void clean_up();
};
