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
#include <cstdio>
#include <sstream>
#include <cctype>

#include "vortex_actions.hpp"

#include "vortex_engine_stats.hpp"
#include "vortex_logs.hpp"
#include "vortex_util.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_compiler.hpp"

class VortexApplication;
class VortexModel;
class VortexCamera;
class ParticleSystem;
class VortexProject;
class ScriptRegistry;
class VortexMonoBehaviour;
class VortexEditor;
class VortexDecal;
class VortexCompiler;

enum class SnapshotState;

struct UIImageData
{
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

namespace VortexGuiHelpers
{
    inline bool DrawHeader(const char* label, bool is_selected)
    {
        if (is_selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.35f, 0.45f, 0.60f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.40f, 0.50f, 0.80f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.35f, 0.45f, 0.55f, 1.00f));
        }

        bool open = ImGui::CollapsingHeader(label);

        if (is_selected) ImGui::PopStyleColor(3);

        return open;
    }

    inline bool DrawComponentNode(const char* label, int flags, const ImVec4& color)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, color);
        bool open = ImGui::TreeNodeEx(label, flags);
        ImGui::PopStyleColor();
        return open;
    }

    inline bool DrawDangerButton(const char* label, const ImVec2& size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.25f, 0.25f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.30f, 0.30f, 1.00f));
        bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return pressed;
    }

    inline bool DrawActionButton(const char* label, const ImVec2& size = ImVec2(0, 0))
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.40f, 0.60f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.70f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.50f, 0.80f, 1.00f));
        bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return pressed;
    }

    inline void DrawTransformRow(const char* label, const char* id, float* data, float step = 0.1f, float min = 0.0f, float max = 0.0f)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", label);

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        if (min != max) ImGui::DragFloat3(id, data, step, min, max);
        else ImGui::DragFloat3(id, data, step);
    }
}

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
    static GLuint folder_icon_tex;
    static GLuint folder_full_icon_tex;
    static GLuint file_icon_tex;
    static GLuint file_cpp_icon_tex;
    static GLuint file_hpp_icon_tex;
    static GLuint file_mtl_icon_tex;
    static GLuint file_obj_icon_tex;
    static GLuint file_txt_icon_tex;

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

    static unsigned int render_scene_fbo;
    static unsigned int render_scene_rbo;
    static unsigned int render_scene_texture;

    static char m_project_to_delete[128];

    static std::unordered_map<std::string, UIImageData> image_thumbnail_cache;
    static std::string active_image_path;

    static void refresh_skybox_list();
    static void gui_set_skybox();

    static void refresh_shader_list();
    static void gui_set_post_processor();

    static void handle_picking(VortexCamera *camera, ImVec2 image_pos);
    static void handle_shortcuts();
    static void snap_to_floor();

    static GLuint load_editor_icon(const char* file_path);
    static GLuint return_file_icon_tex(const std::string &ext_name);
    static GLuint return_folder_icon_tex(const std::filesystem::directory_entry &entry);

public:
    static void setup_scene_fbo(int width, int height);
    static void setup_render_scene_fbo(int width, int height);
    static void destroy_scene();
    static void destroy_render_scene();

    static bool pending_layout_load;
    static std::string pending_ini_data;

    static ImGuizmo::OPERATION m_current_op;
    static bool m_is_using_gizmo;

    static std::vector<std::string> explicit_empty_folders;

    static float scene_width;
    static float scene_height;
    static float render_scene_width;
    static float render_scene_height;

    static bool scene_fbo_initialized;
    static bool render_scene_fbo_initialized;

    static VortexDecal* g_active_decal_for_gizmo;

    static float viewport_width;
    static float viewport_height;
    static float render_viewport_width;
    static float render_viewport_height;

    static bool show_inspector;
    static bool show_camera_info;
    static bool show_creator_window;
    static bool show_engine_stats;
    static bool show_terminal;
    static bool show_skybox_post_process_options;
    static bool show_scene_viewport;
    static bool show_render_scene_viewport;
    static bool show_stack_history_window;
    static bool show_asset_browser;
    static bool show_settings_window;
    static bool show_file_viewer;
    static bool show_image_viewer;

    static char preferred_ide_path[256];

    static std::string current_open_file_path;
    static std::string current_file_content;
    static bool is_current_file_cpp;

    static bool show_create_file_modal;
    static bool show_create_folder_modal;
    static char new_item_name[128];
    static std::string pending_creation_path;

    static bool show_rename_modal;
    static char item_to_rename_old_path[512];
    static char item_to_rename_new_name[256];

    static int m_selected_skybox_idx;
    static int m_selected_shader_idx;

    static char m_new_project_name[128];

    static std::string _vendor_;
    static std::string _renderer_;

    static bool is_scene_view_visible;
    static bool is_render_view_visible;

    static bool is_using_gizmo;
    static ImGuizmo::MODE m_current_guizmo_mode;

    VortexGUI();

    static void build_dockspace();
    static void draw_editor_viewport(VortexCamera* camera);
    static void draw_editor_render_viewport(VortexCamera *camera);
    static void resize_scene_fbo(int width, int height);
    static void resize_render_scene_fbo(int width, int height);
    static void bind_framebuffer();
    static void bind_render_framebuffer();

    static void init(VortexApplication* app, int width, int height);
    static void update();
    static void render();

    static void init_play_mode();

    static void scene_inspector();
    static void inspect_model(VortexModel *model);
    static void inspect_particle_system(ParticleSystem *ps);
    static void draw_model_info_panel(VortexModel* model, int flags);
    static void draw_model_components_panel(VortexModel* model, int flags);
    static void draw_model_transform_panel(VortexModel* model, int flags);
    static void draw_model_actions(VortexModel* model);

    static void draw_main_menu_bar();
    static void draw_project_hub();
    static void project_hub_open_project();
    static void project_hub_new_project();

    static void creator_window();
    static void camera_info(VortexCamera *camera);

    static void post_process_options();
    static void skybox_options();
    static void sync_loaded_environment();

    static void draw_exit_modal();

    static void engine_stats();

    static void draw_terminal();
    static void draw_compiler_modal();

    static void draw_stack_history_window();

    static void draw_asset_browser();

    static void draw_settings_window();

    static void draw_file_viewer();
    static void open_file_in_viewer(const std::string &filepath);

    static void draw_image_viewer();

    static void clean_up();
};
