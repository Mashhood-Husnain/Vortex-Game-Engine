#pragma once

#include <string>
#include <functional>
#include <imgui.h>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <algorithm>

class VortexModel;
class ParticleSystem;
class VortexObjectManager;
class VortexAssetManager;
class VortexApplication;
class VortexDecal;
class VortexProject;

struct LogEntry;

enum class GuiWindowTypes
{
    TerminalWindow,
    InspectorWindow,
    CreatorWindow,
    PostProcessWindow,
    SkyboxWindow,
    EngineStatWindow,
    CameraInfoWindow,
    AssetBrowserWindow,
    SettingsWindow,
};

inline const std::string getGuiWindowTypeName(GuiWindowTypes type)
{
    switch (type)
    {
        case GuiWindowTypes::TerminalWindow: return "Console Terminal"; break;
        case GuiWindowTypes::InspectorWindow: return "Inspector"; break;
        case GuiWindowTypes::CreatorWindow: return "Creator"; break;
        case GuiWindowTypes::PostProcessWindow: return ""; break;
        case GuiWindowTypes::SkyboxWindow: return ""; break;
        case GuiWindowTypes::EngineStatWindow: return ""; break;
        case GuiWindowTypes::CameraInfoWindow: return ""; break;
        case GuiWindowTypes::AssetBrowserWindow: return ""; break;
        case GuiWindowTypes::SettingsWindow: return ""; break;

        default: return "";
    }
}

namespace VortexUI
{
    void LabeledText(const char *label, const char *text_value, float offset=120.0f);
    void InputTextLine(const char *label, char *buffer, size_t buffer_size, std::function<void(const std::string&)> on_change);
    void DragDropTargetBox(const char *payload_id, const char *hint_text, std::function<void(const char*)> on_drop, float height=50.0f);

    namespace Helpers
    {
        bool DrawHeader(const char *label, bool is_selected);
        bool DrawComponentNode(const char *label, int flags, const ImVec4 &colors);
        bool DrawDangerButton(const char *label, const ImVec2 &size=ImVec2(0, 0));
        bool DrawActionButton(const char *label, const ImVec2 &size=ImVec2(0, 0));
        void DrawTransformRow(const char *label, const char *id, float *data, float step=0.1f, float min=0.0f, float max=0.0f);
    }

    namespace Lambda
    {
        void ClampedInputFloat(const char *label, float *v, float step, float step_fast, float v_min, float v_max, const char *format="%.2f");
        void ClampedInputInt(const char *label, int *v, int step, int step_fast, int v_min, int v_max);
        const char *DataGetter(void *data, int idx);
    }
}

struct VortexEditorContext
{
    VortexApplication *application = nullptr;
    char m_new_project_name[128] = "";
};

class VortexImGuiWindow
{
protected:
    virtual void draw_content() = 0;

    virtual ImGuiWindowFlags get_window_flags() const;
    virtual void apply_window_constraints();

public:
    std::string window_name;
    bool is_open;

    VortexImGuiWindow(const std::string &name, bool default_open=false);

    void render();

    virtual ~VortexImGuiWindow() = default;
};

class VortexImGuiManager
{
    inline static std::unordered_map<std::string, std::shared_ptr<VortexImGuiWindow>> windows;
public:
    inline static VortexEditorContext context;

    static void init(VortexApplication *app);
    static void register_window(std::shared_ptr<VortexImGuiWindow> window);
    static void render_all();
    static void toggle_window(const std::string &name);
    static void set_visibility(const std::string &name, const bool is_visible);
    static bool get_visibility(const std::string &name);
};

class VortexTerminalWindow : public VortexImGuiWindow
{
    void render_log_entry(LogEntry &entry);

    bool show_info = true;
    bool show_warning = true;
    bool show_error = true;
    bool show_fatal = true;
    bool auto_scroll = true;

    char subsystem_filter[64] = "";

protected:
    void draw_content() override;
    void apply_window_constraints() override;

public:
    VortexTerminalWindow();
};

class VortexInspectorWindow : public VortexImGuiWindow
{
    std::unordered_set<VortexModel*> m_processed_models;
    std::unordered_set<ParticleSystem*> m_processed_ps;

    void inspect_model(VortexModel *model);
    void draw_model_info_panel(VortexModel *model, int flags);
    void draw_model_components_panel(VortexModel* model, int flags);
    void draw_model_transform_panel(VortexModel* model, int flags);
    void draw_model_actions(VortexModel* model);

    void inspect_particle_system(ParticleSystem* ps);

protected:
    void draw_content() override;
    void apply_window_constraints() override;

public:
    static VortexDecal* g_active_decal_for_gizmo;

    VortexInspectorWindow();
};

class VortexCreatorWindow : public VortexImGuiWindow
{
    bool m_models_scanned = false;
    std::vector<std::pair<std::string, std::string>> cached_user_models;
    std::vector<std::pair<std::string, std::string>> cached_engine_models;

    std::string script_template_path = "templates/Script.cpp.template";

    void create_script(std::string file_path, std::string class_name);

protected:
    void draw_content() override;
    void apply_window_constraints() override;

public:
    VortexCreatorWindow();
};

class PostProcessWindow : public VortexImGuiWindow
{
    bool show_options = true;
    bool m_shaders_loaded = false;

    std::vector<std::string> m_display_names = {};
    int m_selected_shader_idx = 0;

    void refresh_shader_list();
    void set_post_post_processor();

protected:
    void draw_content() override;
    void apply_window_constraints() override;

public:
    PostProcessWindow();
};
