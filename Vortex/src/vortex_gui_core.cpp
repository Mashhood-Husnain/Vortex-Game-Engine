#include "vortex_gui.hpp"
#include "vortex_objectmanager.hpp"
#include "util/vortex_save_load.hpp"
#include "vortex_application.hpp"

#include <thread>
#include <cstdlib>
#include <stb_image.h>

bool VortexGUI::pending_layout_load = false;
std::string VortexGUI::pending_ini_data = "";

GLuint VortexGUI::folder_icon_tex = 0;
GLuint VortexGUI::folder_full_icon_tex;

GLuint VortexGUI::file_icon_tex = 0;
GLuint VortexGUI::file_cpp_icon_tex = 0;
GLuint VortexGUI::file_hpp_icon_tex = 0;
GLuint VortexGUI::file_mtl_icon_tex = 0;
GLuint VortexGUI::file_obj_icon_tex = 0;
GLuint VortexGUI::file_txt_icon_tex = 0;

VortexApplication *VortexGUI::app = nullptr;

int VortexGUI::m_selected_shader_idx = 0;
int VortexGUI::m_selected_skybox_idx = 0;

char VortexGUI::m_new_project_name[128] = "";
char VortexGUI::m_project_to_delete[128] = "";
char VortexGUI::preferred_ide_path[256] = "";

bool VortexGUI::show_inspector;
bool VortexGUI::show_camera_info;
bool VortexGUI::show_creator_window;
bool VortexGUI::show_engine_stats;
bool VortexGUI::show_terminal;
bool VortexGUI::show_skybox_post_process_options;
bool VortexGUI::show_scene_viewport;
bool VortexGUI::show_render_scene_viewport;
bool VortexGUI::show_stack_history_window;
bool VortexGUI::show_asset_browser;
bool VortexGUI::show_settings_window;
bool VortexGUI::show_file_viewer;
bool VortexGUI::show_image_viewer;

std::unordered_map<std::string, UIImageData> VortexGUI::image_thumbnail_cache;
std::string VortexGUI::active_image_path = "";

std::string VortexGUI::current_open_file_path = "";
std::string VortexGUI::current_file_content = "";
bool VortexGUI::is_current_file_cpp = false;

bool VortexGUI::show_create_file_modal = false;
bool VortexGUI::show_create_folder_modal = false;
char VortexGUI::new_item_name[128] = "";
std::string VortexGUI::pending_creation_path = "";

bool VortexGUI::show_rename_modal = false;
char VortexGUI::item_to_rename_old_path[512] = "";
char VortexGUI::item_to_rename_new_name[256] = "";

std::vector<std::string> VortexGUI::explicit_empty_folders = {"Scene"};

std::string VortexGUI::_vendor_ = "";
std::string VortexGUI::_renderer_ = "";

float VortexGUI::scene_width = 1920.0f;
float VortexGUI::scene_height = 1080.0f;
float VortexGUI::render_scene_width = 1920.0f;
float VortexGUI::render_scene_height = 1080.0f;

float VortexGUI::viewport_width = 1920;
float VortexGUI::viewport_height = 1080;

float VortexGUI::render_viewport_width = 1920;
float VortexGUI::render_viewport_height = 1080;

bool VortexGUI::scene_fbo_initialized = true;
bool VortexGUI::render_scene_fbo_initialized = false;

std::set<VortexModel*> VortexGUI::m_processed_models = {nullptr};
std::set<ParticleSystem*> VortexGUI::m_processed_ps = {nullptr};

std::vector<std::string> VortexGUI::m_available_model_names = {};
std::vector<std::string> VortexGUI::m_available_model_paths = {};
bool VortexGUI::m_models_scanned = false;

unsigned int VortexGUI::scene_fbo = 0;
unsigned int VortexGUI::scene_rbo = 0;
unsigned int VortexGUI::scene_texture = 0;

unsigned int VortexGUI::render_scene_fbo = 0;
unsigned int VortexGUI::render_scene_rbo = 0;
unsigned int VortexGUI::render_scene_texture = 0;

ImGuizmo::OPERATION VortexGUI::m_current_op = ImGuizmo::TRANSLATE;
bool VortexGUI::m_is_using_gizmo = false;

std::vector<std::string> VortexGUI::m_shader_files = {};
std::vector<std::string> VortexGUI::m_display_names = {};
bool VortexGUI::m_shaders_loaded = false;

std::vector<std::vector<std::string>> VortexGUI::m_skybox_files = {{}};
std::vector<std::string> VortexGUI::m_skybox_display_names = {};
bool VortexGUI::m_skybox_loaded = false;

bool VortexGUI::is_scene_view_visible = true;
bool VortexGUI::is_render_view_visible = false;

bool VortexGUI::is_using_gizmo = false;
ImGuizmo::MODE VortexGUI::m_current_guizmo_mode = ImGuizmo::LOCAL;

VortexDecal* VortexGUI::g_active_decal_for_gizmo = nullptr;

VortexGUI::VortexGUI()
{

}

void VortexGUI::init(VortexApplication *_app, int width, int height)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;

    ImGuiStyle &style = ImGui::GetStyle();

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;

    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;

    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_WindowBg]       = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    colors[ImGuiCol_ChildBg]        = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]        = ImVec4(0.10f, 0.10f, 0.11f, 0.98f);
    colors[ImGuiCol_Border]         = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);

    colors[ImGuiCol_FrameBg]        = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]  = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_TitleBg]        = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive]  = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);

    colors[ImGuiCol_Header]         = ImVec4(0.20f, 0.20f, 0.22f, 0.00f);
    colors[ImGuiCol_HeaderHovered]  = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive]   = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);

    colors[ImGuiCol_Button]         = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.24f, 0.24f, 0.26f, 1.00f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);

    colors[ImGuiCol_CheckMark]      = ImVec4(0.60f, 0.60f, 0.62f, 1.00f);
    colors[ImGuiCol_SliderGrab]     = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.60f, 0.60f, 0.62f, 1.00f);

    colors[ImGuiCol_Text]           = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
    colors[ImGuiCol_TextDisabled]   = ImVec4(0.45f, 0.45f, 0.48f, 1.00f);

    colors[ImGuiCol_Separator]      = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]=ImVec4(0.22f, 0.22f, 0.24f, 1.00f);

    app = _app;
    setup_scene_fbo(width, height);
    scene_fbo_initialized = true;

    ImGui_ImplGlfw_InitForOpenGL(app->get_window_ptr(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    folder_icon_tex = load_editor_icon("assets/images/icons/folder.png");
    folder_full_icon_tex = load_editor_icon("assets/images/icons/folder_full.png");

    file_icon_tex = load_editor_icon("assets/images/icons/file.png");
    file_cpp_icon_tex = load_editor_icon("assets/images/icons/file_cpp.png");
    file_hpp_icon_tex = load_editor_icon("assets/images/icons/file_hpp.png");
    file_mtl_icon_tex = load_editor_icon("assets/images/icons/file_mtl.png");
    file_obj_icon_tex = load_editor_icon("assets/images/icons/file_obj.png");
    file_txt_icon_tex = load_editor_icon("assets/images/icons/file_txt.png");
}

void VortexGUI::update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (pending_layout_load)
    {
        ImGui::LoadIniSettingsFromMemory(pending_ini_data.c_str(), pending_ini_data.size());
        pending_layout_load = false;
        pending_ini_data.clear();
        VORTEX_INFO("[GUI] Restored window layout from save file.");
    }

    m_processed_models.clear();
    m_processed_ps.clear();
}

void VortexGUI::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void VortexGUI::draw_exit_modal()
{
    if (!app->is_exit_modal_open()) return;

    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.12f, 0.12f, 0.14f, 1.00f));
    ImGui::OpenPopup("Exit Vortex Engine");

    app->show_mouse(true);

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Exit Vortex Engine", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Spacing();

        if (app->has_unsaved())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Warning: You have unsaved changes!");
            ImGui::Text("Are you sure you want to exit? Unsaved progress will be lost.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Project Name:");
            ImGui::SetNextItemWidth(300.0f);
            ImGui::InputText("##ProjectName", m_new_project_name, IM_ARRAYSIZE(m_new_project_name));

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("Save & Exit", ImVec2(120, 30)))
            {
                VortexProject::take_snapshot(SnapshotState::SAVE, app);

                VortexProject::create_backup(m_new_project_name);

                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }

            ImGui::SameLine();

            if (ImGui::Button("Exit Without Saving", ImVec2(160, 30)))
            {
                VortexProject::create_backup(m_new_project_name);

                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(100, 30)))
            {
                ImGui::CloseCurrentPopup();
                app->toggle_exit_modal(false);
            }
        }
        else
        {
            ImGui::Text("Project is saved.");
            ImGui::Text("Are you sure you want to exit Vortex Engine?");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Yes, Exit", ImVec2(120, 30)))
            {
                VortexProject::create_backup(m_new_project_name);

                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(app->get_window_ptr(), GLFW_TRUE);
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 30)))
            {
                ImGui::CloseCurrentPopup();
                app->toggle_exit_modal(false);
            }
        }

        ImGui::Spacing();
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor();
}

GLuint VortexGUI::load_editor_icon(const char* file_path)
{
    int width, height, channels;
    unsigned char *data = stbi_load(file_path, &width, &height, &channels, 4);
    if (!data)
    {
        VORTEX_WARN("[GUI] Failed to load editor icon: ", file_path);
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return texture_id;
}

void VortexGUI::init_play_mode()
{
    show_inspector = false;
    show_camera_info = false;
    show_creator_window = false;
    show_engine_stats = false;
    show_terminal = false;
    show_skybox_post_process_options = false;
    show_scene_viewport = false;
    show_stack_history_window = false;
    show_asset_browser = false;
    show_settings_window = false;
    show_file_viewer = false;
    show_image_viewer = false;

    show_render_scene_viewport = true;
    if (!render_scene_fbo_initialized)
    {
        setup_render_scene_fbo(render_scene_width, render_scene_height);
        render_scene_fbo_initialized = true;
    }

    for (VortexModel* model : VortexObjectManager::selected_models)
    {
        if (model) model->is_selected = false;
    }
    VortexObjectManager::selected_models.clear();

    const GLubyte *vendor_ptr = glGetString(GL_VENDOR);
    const GLubyte *renderer_ptr = glGetString(GL_RENDERER);
    _vendor_ = vendor_ptr ? reinterpret_cast<const char*>(vendor_ptr) : "Unknown Vendor";
    _renderer_  = renderer_ptr ? reinterpret_cast<const char*>(renderer_ptr) : "Unknown Renderer";
}

void VortexGUI::clean_up()
{
    VORTEX_INFO("[GUI] Cleaning up UI subsystem resources...");

    m_processed_models.clear();
    m_processed_ps.clear();

    m_shader_files.clear();
    m_display_names.clear();
    m_skybox_files.clear();
    m_skybox_display_names.clear();
    m_available_model_names.clear();
    m_available_model_paths.clear();

    if (scene_texture != 0)
    {
        glDeleteTextures(1, &scene_texture);
        scene_texture = 0;
    }

    if (render_scene_texture != 0)
    {
        glDeleteTextures(1, &render_scene_texture);
        render_scene_texture = 0;
    }

    if (scene_rbo != 0)
    {
        glDeleteRenderbuffers(1, &scene_rbo);
        scene_rbo = 0;
    }

    if (render_scene_rbo != 0)
    {
        glDeleteRenderbuffers(1, &render_scene_rbo);
        render_scene_rbo = 0;
    }

    if (scene_fbo != 0)
    {
        glDeleteFramebuffers(1, &scene_fbo);
        scene_fbo = 0;
    }

    if (render_scene_fbo != 0)
    {
        glDeleteFramebuffers(1, &render_scene_fbo);
        render_scene_fbo = 0;
    }

    for (auto const& [path, data] : image_thumbnail_cache)
    {
        if (data.id != 0) glDeleteTextures(1, &data.id);
    }
    image_thumbnail_cache.clear();

    glDeleteTextures(1, &folder_icon_tex);
    glDeleteTextures(1, &folder_full_icon_tex);

    glDeleteTextures(1, &file_icon_tex);
    glDeleteTextures(1, &file_cpp_icon_tex);
    glDeleteTextures(1, &file_hpp_icon_tex);
    glDeleteTextures(1, &file_mtl_icon_tex);
    glDeleteTextures(1, &file_obj_icon_tex);
    glDeleteTextures(1, &file_txt_icon_tex);

    m_shaders_loaded = false;
    m_skybox_loaded = false;
    m_models_scanned = false;
    m_is_using_gizmo = false;

    app = nullptr;
}
