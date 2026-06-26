/*
 * File: vortex_application.cpp
 * Project: VortexEngine
 * Description: Implementation of window
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_application.hpp"

bool VortexApplication::has_unsaved_changes = false;

void VortexApplication::load_splash_screen()
{
    if (!enable_splash_screen)
    {
        is_playing_splash = false;
        return;
    }

    int channels;
    unsigned char* data = stbi_load("assets/images/branding/vortex_logo.png", &splash_width, &splash_height, &channels, 4);

    if (!data)
    {
        VORTEX_ERROR("Splash screen logo not found!");
        is_playing_splash = false;
        return;
    }

    glGenTextures(1, &splash_texture);
    glBindTexture(GL_TEXTURE_2D, splash_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, splash_width, splash_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
}

void VortexApplication::draw_splash_overlay()
{
    if (!is_playing_splash) return;

    splash_timer += GLOBAL::deltaTime;
    float total_duration = 4.5f;
    float fade_duration = 2.0f;

    if (splash_timer >= total_duration)
    {
        is_playing_splash = false;
        glDeleteTextures(1, &splash_texture);
        return;
    }

    float logo_alpha = 1.0f;
    if (splash_timer < fade_duration) logo_alpha = splash_timer / fade_duration;
    else if (splash_timer > total_duration - fade_duration) logo_alpha = (total_duration - splash_timer) / fade_duration;

    float bg_alpha = 1.0f;
    if (splash_timer > total_duration - fade_duration) bg_alpha = (total_duration - splash_timer) / fade_duration;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImDrawList* draw_list = ImGui::GetForegroundDrawList(viewport);

    ImU32 bg_color = IM_COL32(15, 15, 18, (int)(bg_alpha * 255.0f));
    draw_list->AddRectFilled(ImVec2(-5000.0f, -5000.0f), ImVec2(10000.0f, 10000.0f), bg_color);

    float scaleX = viewport->Size.x / (float)splash_width;
    float scaleY = viewport->Size.y / (float)splash_height;
    float scale = std::max(scaleX, scaleY);

    float display_width = splash_width * scale;
    float display_height = splash_height * scale;

    ImVec2 center = ImVec2(
        viewport->Pos.x + (viewport->Size.x - display_width) * 0.5f,
        viewport->Pos.y + (viewport->Size.y - display_height) * 0.5f
    );

    ImU32 tint_color = IM_COL32(255, 255, 255, (int)(logo_alpha * 255.0f));

    draw_list->AddImage(
        (ImTextureID)(intptr_t)splash_texture,
        center,
        ImVec2(center.x + display_width, center.y + display_height),
        ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
        tint_color
    );
}

void VortexApplication::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<VortexApplication*>(glfwGetWindowUserPointer(window));
    if (app && width > 0 && height > 0)
    {
        app->default_window_width = static_cast<float>(width);
        app->default_window_height = static_cast<float>(height);
    }
}

void VortexApplication::request_exit()
{
    if (current_state == EngineState::PROJECT_HUB)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    std::string project_name = std::string(VortexGUI::m_new_project_name);
    Snapshot snapshot = {
        project_name,
        VortexObjectManager::active_models,
        VortexObjectManager::active_particlesystems,
        VortexGUI::m_selected_skybox_idx,
        VortexGUI::m_selected_shader_idx,
        VortexGUI::explicit_empty_folders,
        this
    };

    has_unsaved_changes = !VortexProject::check_save_state(&snapshot);
    show_exit_modal = true;

    glfwSetWindowShouldClose(window, GLFW_FALSE);
}

void VortexApplication::window_close_callback(GLFWwindow* window)
{
    VortexApplication* app = (VortexApplication*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->request_exit();
    }
}

void VortexApplication::check_key_press()
{
    if (is_playing_splash) return;
    if (current_state == EngineState::PROJECT_HUB) return;

    if (VortexKeyboard::get_key_down("ESCAPE"))
    {
        if (current_state == EngineState::PLAY) exit_play_mode();
        else request_exit();
    }

    if (current_state == EngineState::EDITOR)
    {
        if (VortexKeyboard::get_key_down("DELETE") && !VortexObjectManager::selected_models.empty()) VortexObjectManager::delete_selected_models();

        if (VortexKeyboard::get_key("LEFTCONTROL"))
        {
            if (VortexKeyboard::get_key("LEFTSHIFT"))
            {
                if (VortexKeyboard::get_key_down("S")) VortexGUI::show_scene_viewport = !VortexGUI::show_scene_viewport;
                if (VortexKeyboard::get_key_down("R")) VortexGUI::show_render_scene_viewport = !VortexGUI::show_render_scene_viewport;
                if (VortexKeyboard::get_key_down("I")) VortexGUI::show_settings_window = !VortexGUI::show_settings_window;

                if (VortexKeyboard::get_key_down("O")) VortexGUI::project_hub_open_project();
                if (VortexKeyboard::get_key_down("N")) VortexGUI::project_hub_new_project();
            }
            else
            {
                if (VortexKeyboard::get_key_down("S")) VortexProject::take_snapshot(SnapshotState::SAVE, this, std::string(VortexGUI::m_new_project_name));
                if (VortexKeyboard::get_key_down("R")) enter_play_mode();

                if (VortexKeyboard::get_key_down("O")) VortexGUI::show_skybox_post_process_options = !VortexGUI::show_skybox_post_process_options;
                if (VortexKeyboard::get_key_down("I")) VortexGUI::show_inspector = !VortexGUI::show_inspector;
            }

            if (!VortexGUI::is_using_gizmo)
            {
                if (VortexKeyboard::get_key_down("Z", 0.5f, true)) ActionManager::undo();
                if (VortexKeyboard::get_key_down("Y", 0.5f, true)) ActionManager::redo();
            }

            if (VortexKeyboard::get_key_down("P")) VortexGUI::show_terminal = !VortexGUI::show_terminal;
            if (VortexKeyboard::get_key_down("X")) VortexGUI::show_camera_info = !VortexGUI::show_camera_info;
            if (VortexKeyboard::get_key_down("W")) VortexGUI::show_creator_window = !VortexGUI::show_creator_window;
            if (VortexKeyboard::get_key_down("E")) VortexGUI::show_engine_stats = !VortexGUI::show_engine_stats;
            if (VortexKeyboard::get_key_down("H")) VortexGUI::show_stack_history_window = !VortexGUI::show_stack_history_window;
            if (VortexKeyboard::get_key_down("A")) VortexGUI::show_asset_browser = !VortexGUI::show_asset_browser;
            if (VortexKeyboard::get_key_down("F")) VortexGUI::show_file_viewer = !VortexGUI::show_file_viewer;
            if (VortexKeyboard::get_key_down("B")) VortexGUI::show_image_viewer = !VortexGUI::show_image_viewer;

            if (VortexKeyboard::get_key_down("C")) VortexCompiler::trigger_compile();
            if (VortexKeyboard::get_key_down("D")) VortexObjectManager::duplicate_selected_models();
        }
        else
        {
            if (VortexKeyboard::get_key_down("Z")) show_wireframe = !show_wireframe;
            if (VortexKeyboard::get_key_down("M") && (VortexGUI::show_scene_viewport || VortexGUI::show_render_scene_viewport)) show_mouse(!show_mouse_cursor);
            if (VortexKeyboard::get_key_down("F"))
            {
                is_fullscreen = !is_fullscreen;
                change_window_size();
            }
        }
    }
    else
    {
        if (VortexKeyboard::get_key_down("TAB")) VortexGUI::show_engine_stats= !VortexGUI::show_engine_stats;
    }
}

void VortexApplication::setup_world_axis_buffers()
{
    glGenVertexArrays(1, &world_axisVAO);
    glGenBuffers(1, &world_axisVBO);
    glBindVertexArray(world_axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, world_axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLOBAL::DEFAULT_VERTICES::WORLD_AXES_VERTICES), GLOBAL::DEFAULT_VERTICES::WORLD_AXES_VERTICES, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
}

// void glfw_error_callback(int error, const char* description)
// {
//     std::cerr << "[GLFW ERROR " << error << "]: " << description << std::endl;
// }

VortexApplication::VortexApplication(std::string window_name)
{
    // glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
    {
        VORTEX_ERROR("Failed to initialize GLFW");
        exit(EXIT_FAILURE);
    }

    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    int work_x, work_y, work_width, work_height;
    glfwGetMonitorWorkarea(primary_monitor, &work_x, &work_y, &work_width, &work_height);

    default_window_width = static_cast<float>(work_width);
    default_window_height = static_cast<float>(work_height);

    stored_window_width = default_window_width;
    stored_window_height = default_window_height;

    stored_window_x_pos = static_cast<float>(work_x);
    stored_window_y_pos = static_cast<float>(work_y);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    this->window_name = window_name + " - " + GLOBAL::VORTEX_VERSION;

    editor_camera = new VortexCamera(glm::vec3(15.0f, 2.0f, 1.0f));
    editor_camera->look_at(glm::vec3(0.0f, 2.0f, 0.0f));
    camera = editor_camera;
    render_camera = editor_camera;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    window = glfwCreateWindow(
        static_cast<int>(default_window_width),
        static_cast<int>(default_window_height),
        this->window_name.c_str(),
        nullptr,
        nullptr
    );

    if (window == nullptr)
    {
        VORTEX_ERROR("Failed to create GLFW window");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);

    if(glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        VORTEX_INFO("[INPUT] Hardware Raw Mouse Motion Enabled");
    }

    unsigned char pixels[16 * 16 * 4] = {0};
    GLFWimage image = {
        .width = 16,
        .height = 16,
        .pixels = pixels,
    };
    m_invisible_cursor = glfwCreateCursor(&image, 0, 0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        VORTEX_ERROR("Failed to initialize GLAD");
        exit(EXIT_FAILURE);
    }

    int start_width, start_height;
    glfwGetFramebufferSize(window, &start_width, &start_height);

    worldaxis_shader = new VortexShader("shaders/world_axis.vert", "shaders/world_axis.frag");
    setup_world_axis_buffers();

    // shadow_manager = new ShadowManager();
    environment_grid = new VortexGrid();

    VortexObjectManager::init();
    VortexKeyboard::init(window);
    VortexMouse::init(window);
    VortexAudio::init();
    VortexDebugRenderer::get().init();

    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    change_window_size();
    srand(static_cast<unsigned int>(time(0)));

    if (this->camera)
    {
        float aspect_ratio = static_cast<float>(start_width) / static_cast<float>(start_height);
        this->camera->set_aspect_ratio(aspect_ratio);
    }

    if (this->render_camera)
    {
        float aspect_ratio = static_cast<float>(start_width) / static_cast<float>(start_height);
        this->render_camera->set_aspect_ratio(aspect_ratio);
    }

    VortexGUI::init(this, start_width, start_height);
}

VortexApplication::~VortexApplication()
{
    delete worldaxis_shader;
    // delete shadow_manager;
    delete skybox;
    delete environment_grid;
    delete editor_camera;

    if (post_processor) delete post_processor;

    ActionManager::clear_stack_history();
    VortexObjectManager::clean_up();
    VortexAssetManager::clean_up();

    VortexAudio::clean_up();
    VortexDebugRenderer::get().clean_up();
    VortexGUI::clean_up();
    VortexProject::clean_playmode_backups();
    VortexCompiler::clean_up();

    worldaxis_shader = nullptr;
    // shadow_manager = nullptr;
    window = nullptr;
    camera = nullptr;
    editor_camera = nullptr;
    render_camera = nullptr;
    post_processor = nullptr;
    skybox = nullptr;
    environment_grid = nullptr;

    if (window) glfwDestroyWindow(window);
    glfwTerminate();

    VORTEX_INFO("[ENGINE] Successfully Closed Application!");
}

void VortexApplication::change_window_size()
{
    if (is_fullscreen)
    {
        GLFWmonitor* monitor = glfwGetWindowMonitor(window);
        if (!monitor)
        {
            int window_x, window_y;
            glfwGetWindowPos(window, &window_x, &window_y);

            int width, height;
            glfwGetWindowSize(window, &width, &height);
            stored_window_width = static_cast<float>(width);
            stored_window_height = static_cast<float>(height);

            stored_window_x_pos = static_cast<float>(window_x);
            stored_window_y_pos = static_cast<float>(window_y);

            int monitor_count;
            GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
            monitor = monitors[0];

            for (int i = 0; i < monitor_count; i++)
            {
                int mx, my, mw, mh;
                glfwGetMonitorWorkarea(monitors[i], &mx, &my, &mw, &mh);
                if (window_x >= mx && window_x < mx + mw && window_y >= my && window_y < my + mh)
                {
                    monitor = monitors[i];
                    break;
                }
            }
        }

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

        default_window_width = static_cast<float>(mode->width);
        default_window_height = static_cast<float>(mode->height);
    }
    else
    {
        glfwSetWindowMonitor(
            window,
            nullptr,
            static_cast<int>(stored_window_x_pos),
            static_cast<int>(stored_window_y_pos),
            static_cast<int>(stored_window_width),
            static_cast<int>(stored_window_height),
            0
        );

        default_window_width  = stored_window_width;
        default_window_height = stored_window_height;
    }

    glfwFocusWindow(window);
    glfwSwapInterval(1);
    first_mouse = true;
}

void VortexApplication::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    auto* app = static_cast<VortexApplication*>(glfwGetWindowUserPointer(window));

    if (app)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (app->first_mouse)
        {
            app->mouse_last_x = xpos;
            app->mouse_last_y = ypos;
            app->first_mouse = false;
        }

        float xoffset = xpos - app->mouse_last_x;
        float yoffset = app->mouse_last_y - ypos;

        app->mouse_last_x = xpos;
        app->mouse_last_y = ypos;
        if (!app->show_mouse_cursor)
        {
            if (app->current_state == EngineState::EDITOR)
            {
                app->editor_camera->processMouseMovement(xoffset, yoffset);
            }
            else if (app->current_state == EngineState::PLAY && app->camera)
            {
                app->camera->processMouseMovement(xoffset, yoffset);
            }
        }
    }
}

void VortexApplication::run(std::function<void()> draw_callback)
{
    VORTEX_INFO("VORTEX ENGINE RUNNING ON:");
    VORTEX_INFO("VENDOR:   ", VortexGUI::_vendor_);
    VORTEX_INFO("RENDERER: ", VortexGUI::_renderer_);

    glfwShowWindow(window);

    load_splash_screen();

    last_frame = static_cast<float>(glfwGetTime());

    while(!glfwWindowShouldClose(window))
    {
        VortexCompiler::check_for_hot_reload(this);

        glfwPollEvents();
        check_key_press();

        float currentFrame = static_cast<float>(glfwGetTime());
        GLOBAL::deltaTime = currentFrame - last_frame;
        last_frame = currentFrame;

        VortexGUI::update();

        if (current_state == EngineState::PROJECT_HUB)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, static_cast<GLsizei>(default_window_width), static_cast<GLsizei>(default_window_height));

            glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            show_mouse(true);

            VortexGUI::draw_project_hub();
        }
        else
        {
            if (current_state != EngineState::PROJECT_HUB) VortexCompiler::update(this);

            if (!show_mouse_cursor && current_state == EngineState::EDITOR &&
                (
                    (VortexGUI::is_scene_view_visible && VortexGUI::show_scene_viewport) ||
                    (VortexGUI::is_render_view_visible && VortexGUI::show_render_scene_viewport)
                )
            )
            {
                editor_camera->check_camera_movement();
            }

            if ((!VortexGUI::show_scene_viewport && !VortexGUI::show_render_scene_viewport) && !show_mouse_cursor)
            {
                show_mouse(true);
            }

            if (current_state == EngineState::PLAY)
            {
                VortexObjectManager::update_scripts();
                VortexObjectManager::update_physics();
            }

            VortexObjectManager::update_particles();

            if (VortexGUI::scene_fbo_initialized && VortexGUI::viewport_width > 0.0f && VortexGUI::viewport_height > 0.0f &&
            (VortexGUI::viewport_height != VortexGUI::scene_height || VortexGUI::viewport_width != VortexGUI::scene_width))
            {
                VortexGUI::resize_scene_fbo(
                    static_cast<int>(VortexGUI::viewport_width),
                    static_cast<int>(VortexGUI::viewport_height)
                );
            }

            if (VortexGUI::render_scene_fbo_initialized && VortexGUI::render_viewport_width > 0.0f && VortexGUI::render_viewport_height > 0.0f &&
            (VortexGUI::render_viewport_height != VortexGUI::render_scene_height || VortexGUI::render_viewport_width != VortexGUI::render_scene_width))
            {
                VortexGUI::resize_render_scene_fbo(
                    static_cast<int>(VortexGUI::render_viewport_width),
                    static_cast<int>(VortexGUI::render_viewport_height)
                );

                if (post_processor) post_processor->resize(
                    static_cast<int>(VortexGUI::render_scene_width),
                    static_cast<int>(VortexGUI::render_scene_height)
                );
            }

            // First pass (Scene View)
            if (VortexGUI::is_scene_view_visible && VortexGUI::show_scene_viewport)
            {
                if (!VortexGUI::scene_fbo_initialized)
                {
                    int safe_w = std::max(1, static_cast<int>(VortexGUI::viewport_width));
                    int safe_h = std::max(1, static_cast<int>(VortexGUI::viewport_height));

                    VortexGUI::setup_scene_fbo(safe_w, safe_h);

                    VortexGUI::scene_width = safe_w;
                    VortexGUI::scene_height = safe_h;

                    VortexGUI::scene_fbo_initialized = true;
                }

                VortexGUI::bind_framebuffer();

                glViewport(0, 0, static_cast<GLsizei>(VortexGUI::scene_width), static_cast<GLsizei>(VortexGUI::scene_height));

                if (VortexGUI::scene_height > 0 && editor_camera)
                {
                    editor_camera->set_aspect_ratio(VortexGUI::scene_width / VortexGUI::scene_height);
                }

                if (!skybox) glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

                glStencilMask(0xFF);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                if (skybox) skybox->draw(editor_camera);
                draw_callback();

                VortexObjectManager::draw(*editor_camera, show_wireframe, false);
                VortexObjectManager::check_object_status();

                VortexDebugRenderer::get().render(editor_camera);

                if (current_state == EngineState::EDITOR)
                {
                    environment_grid->draw(*editor_camera);
                }
            }
            else if ((!VortexGUI::show_scene_viewport || !VortexGUI::is_scene_view_visible) && VortexGUI::scene_fbo_initialized)
            {
                // destroy scene FBO

                VortexGUI::destroy_scene();
                VortexGUI::scene_fbo_initialized = false;
            }

            // Second Pass (Render View)
            if (VortexGUI::is_render_view_visible && VortexGUI::show_render_scene_viewport)
            {
                if (!VortexGUI::render_scene_fbo_initialized)
                {
                    int safe_w = std::max(1, static_cast<int>(VortexGUI::render_viewport_width));
                    int safe_h = std::max(1, static_cast<int>(VortexGUI::render_viewport_height));

                    VortexGUI::setup_render_scene_fbo(safe_w, safe_h);

                    VortexGUI::render_scene_width = safe_w;
                    VortexGUI::render_scene_height = safe_h;

                    VortexGUI::render_scene_fbo_initialized = true;
                }

                if (post_processor)
                {
                    post_processor->begin();
                }
                else
                {
                    VortexGUI::bind_render_framebuffer();
                }

                glViewport(0, 0, static_cast<GLsizei>(VortexGUI::render_scene_width), static_cast<GLsizei>(VortexGUI::render_scene_height));

                if (VortexGUI::render_scene_height > 0 && render_camera)
                {
                    render_camera->set_aspect_ratio(VortexGUI::render_scene_width / VortexGUI::render_scene_height);
                }

                if (!skybox) glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                if (skybox) skybox->draw(render_camera);

                VortexObjectManager::draw(*render_camera, false, true);

                if (post_processor)
                {
                    post_processor->end();
                    VortexGUI::bind_render_framebuffer();
                    glViewport(0, 0, static_cast<GLsizei>(VortexGUI::render_scene_width), static_cast<GLsizei>(VortexGUI::render_scene_height));
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    post_processor->draw(currentFrame);
                }
            }
            else if ((!VortexGUI::show_render_scene_viewport || !VortexGUI::is_render_view_visible) && VortexGUI::render_scene_fbo_initialized)
            {
                // destroy render FBO

                VortexGUI::destroy_render_scene();
                VortexGUI::render_scene_fbo_initialized = false;
            }

            // Third pass (IMGUI View)
            VortexUIManager::render_ui();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, static_cast<GLsizei>(default_window_width), static_cast<GLsizei>(default_window_height));

            glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            VortexGUI::build_dockspace();
            VortexGUI::draw_main_menu_bar();
            VortexGUI::draw_asset_browser();
            VortexGUI::engine_stats();
            VortexGUI::camera_info(camera);
            VortexGUI::post_process_options();
            VortexGUI::skybox_options();
            VortexGUI::creator_window();
            VortexGUI::scene_inspector();
            VortexGUI::draw_terminal();
            VortexGUI::draw_stack_history_window();
            VortexGUI::draw_settings_window();
            VortexGUI::draw_file_viewer();
            VortexGUI::draw_image_viewer();

            VortexGUI::draw_compiler_modal();
            VortexGUI::draw_exit_modal();

            VortexGUI::draw_editor_render_viewport(render_camera);
            VortexGUI::draw_editor_viewport(editor_camera);
        }

        if (is_playing_splash)
        {
            draw_splash_overlay();
        }

        VortexGUI::render();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        VortexKeyboard::update();
        VortexMouse::update();
        VortexAudio::update();
        VortexDebugRenderer::get().update();

        float pre_swap_time = static_cast<float>(glfwGetTime());
        raw_render_time = pre_swap_time - last_frame;

        glfwSwapBuffers(window);
    }
}

float VortexApplication::get_true_fps() const
{
    return raw_render_time > 0.0f ? (1.0f / raw_render_time) : 0.0f;
}

float VortexApplication::get_vsync_fps() const
{
    return GLOBAL::deltaTime > 0.0f ? (1.0f / GLOBAL::deltaTime) : 0.0f;
}

void VortexApplication::set_post_processor(PostProcessor *post_processor)
{
    if (this->post_processor) delete this->post_processor;

    this->post_processor = post_processor;
    if (this->post_processor)
    {
        this->post_processor->resize(
            static_cast<int>(VortexGUI::render_scene_width),
            static_cast<int>(VortexGUI::render_scene_height)
        );
    }
}

void VortexApplication::set_skybox(VortexSkybox *skybox)
{
    delete this->skybox;
    this->skybox = skybox;
}

void VortexApplication::show_mouse(bool status)
{
    show_mouse_cursor = status;

    ImGuiIO &io = ImGui::GetIO();

    if (show_mouse_cursor)
    {
        glfwSetCursor(window, NULL);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }
    else
    {
        glfwSetCursor(window, m_invisible_cursor);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    }
}

void VortexApplication::enter_play_mode()
{
    if (current_state == EngineState::PLAY) return;

    VORTEX_INFO("[ENGINE] Entering Play Mode...");

    std::string project_name = VortexGUI::m_new_project_name;
    VortexProject::take_snapshot(SnapshotState::SAVE, this, "temp_playmode_backup_" + project_name);

    set_state(EngineState::PLAY);
    show_mouse(false);
    toggle_wireframe(false);

    VortexGUI::init_play_mode();
    VortexObjectManager::init_scripts();
}

void VortexApplication::exit_play_mode()
{
    VORTEX_INFO("[ENGINE] Exiting Play Mode...");

    VortexObjectManager::clean_scripts();

    set_state(EngineState::EDITOR);
    show_mouse(true);

    camera = editor_camera;
    render_camera = editor_camera;

    VortexUIManager::cleanup();

    VortexObjectManager::clear_scene();

    std::string project_name = VortexGUI::m_new_project_name;
    VortexProject::take_snapshot(SnapshotState::LOAD, this, "temp_playmode_backup_" + project_name);
}

GLFWwindow* VortexApplication::get_window_ptr() const { return window; }
float VortexApplication::get_width() const { return default_window_width; }
float VortexApplication::get_height() const { return default_window_height; }

bool VortexApplication::is_mouse_visible() { return show_mouse_cursor; }

EngineState VortexApplication::get_state() const { return current_state; }
bool VortexApplication::is_wireframe_enabled() const { return show_wireframe; }
bool VortexApplication::is_exit_modal_open() const { return show_exit_modal; }
bool VortexApplication::has_unsaved() const { return has_unsaved_changes; }

VortexCamera *VortexApplication::get_camera() const { return render_camera; }
VortexCamera *VortexApplication::get_editor_camera() const { return editor_camera; }
// ShadowManager *VortexApplication::get_shadow_manager() const { return shadow_manager; }

void VortexApplication::set_camera(VortexCamera *camera)
{
    this->render_camera = camera;
    this->camera = camera;
}

void VortexApplication::set_state(EngineState state) { current_state = state; }
void VortexApplication::toggle_wireframe(bool enable) { show_wireframe = enable; }
void VortexApplication::toggle_exit_modal(bool show) { show_exit_modal = show; }

void VortexApplication::mark_unsaved_changes() { has_unsaved_changes = true; }
void VortexApplication::clear_unsaved_changes() {has_unsaved_changes = false; }
