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
    VortexApplication *app;

    std::set<VortexModel*> m_processed_models;

    std::set<ParticleSystem*> m_processed_ps;

    std::vector<std::string> m_shader_files;
    std::vector<std::string> m_display_names;
    bool m_shaders_loaded = false;

    std::vector<std::vector<std::string>> m_skybox_files;
    std::vector<std::string> m_skybox_display_names;
    bool m_skybox_loaded = false;

    ImVec2 m_scene_pos = ImVec2(0, 0);
    ImVec2 m_scene_size = ImVec2(0, 0);

    bool m_scene_was_collapsed = true;
    bool m_creator_was_collapsed = true;
    bool m_force_scene_collapse = true;
    bool m_force_creator_collapse = true;

    std::vector<std::string> m_available_model_names;
    std::vector<std::string> m_available_model_paths;
    bool m_models_scanned = false;

    unsigned int scene_fbo = 0;
    unsigned int scene_rbo = 0;
    unsigned int scene_texture = 0;

    char m_new_project_name[128] = "";

    void refresh_skybox_list();
    void gui_set_skybox();

    void refresh_shader_list();
    void gui_set_post_processor();

    void handle_picking(VortexCamera *camera, ImVec2 image_pos);
    void handle_shortcuts();
    void snap_to_floor();

    void setup_scene_fbo(int width, int height);
public:
    std::set<VortexModel*> m_selected_models;
    ImGuizmo::OPERATION m_current_op = ImGuizmo::TRANSLATE;
    bool m_is_using_gizmo = false;

    static std::vector<std::string> explicit_empty_folders;

    int scene_width = 1920;
    int scene_height = 1080;

    int viewport_width = 1920;
    int viewport_height = 1080;

    static char save_project_name[128];

    bool show_inspector = true;
    bool show_camera_info = true;
    bool show_creator_window = true;
    bool show_engine_stats = true;
    bool show_terminal = false;
    bool show_tool_window = true;
    bool show_skybox_post_process_options = true;    

    static int m_selected_skybox_idx;
    static int m_selected_shader_idx;

    std::string _vendor_;
    std::string _renderer_;

    VortexGUI();
    ~VortexGUI();

    void build_dockspace();
    void draw_editor_viewport(float deltaTime, VortexCamera* camera);
    void resize_scene_fbo(int width, int height);
    void bind_framebuffer();

    void init(VortexApplication* app, int width, int height);
    void update();
    void render();

    void scene_inspector();
    void inspect_model(VortexModel *model);
    void inspect_particle_system(ParticleSystem *ps);
    void draw_model_info_panel(VortexModel* model, int flags);
    void draw_model_components_panel(VortexModel* model, int flags);
    void draw_model_transform_panel(VortexModel* model, int flags);
    void draw_model_actions(VortexModel* model);

    void draw_main_menu_bar();
    void draw_project_hub();

    void creator_window();
    void camera_info(VortexCamera *camera);

    void post_process_options();
    void skybox_options();

    void draw_exit_modal();

    void engine_stats();

    void draw_terminal();
    void draw_compiler_modal();
};
