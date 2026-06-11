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
    unsigned char* data = stbi_load("assets/branding/vortex_logo.png", &splash_width, &splash_height, &channels, 4);

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

void VortexApplication::draw_splash_overlay(float dt)
{
    if (!is_playing_splash) return;

    splash_timer += dt;
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

GLFWmonitor* VortexApplication::get_current_monitor(GLFWwindow* window)
{
    int nmonitors;
    int wx, wy, ww, wh;
    int mx, my, mw, mh;
    int overlap, bestoverlap = 0;
    GLFWmonitor* bestmonitor = nullptr;
    GLFWmonitor**monitors;
    const GLFWvidmode* mode;

    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (int i = 0; i < nmonitors; i++)
    {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        overlap =
            std::max(0, std::min(wx+ww, mx+mw) - std::max(wx, mx)) *
            std::max(0, std::min(wy+wh, my+mh) - std::max(wy, my));

        if (bestoverlap < overlap)
        {
            bestoverlap = overlap;
            bestmonitor = monitors[i];
        }
    }
    return bestmonitor;
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

void duplicate_models()
{
    std::vector<VortexModel*> newly_cloned_models;

    for (VortexModel *current_model : VortexGUI::m_selected_models)
    {
        VortexModel *cloned_model = current_model->clone();
        VortexObjectManager::active_models.push_back(cloned_model);

        newly_cloned_models.push_back(cloned_model);

        current_model->is_selected = false;

        ActionManager::push_action(new ActionCreate(cloned_model));
    }

    VortexGUI::m_selected_models.clear();

    for (VortexModel * clone : newly_cloned_models)
    {
        clone->is_selected = true;
        VortexGUI::m_selected_models.insert(clone);
    }
}

void delete_models()
{
    for (VortexModel *model : VortexGUI::m_selected_models)
    {
        model->is_selected = false;

        ActionDelete *delete_command = new ActionDelete(model);
        delete_command->redo();

        ActionManager::push_action(delete_command);
    }

    VortexGUI::m_selected_models.clear();
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
        if (VortexKeyboard::get_key_down("DELETE") && !VortexGUI::m_selected_models.empty()) delete_models();

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
                if (VortexKeyboard::get_key_down("Z")) ActionManager::undo();
                if (VortexKeyboard::get_key_down("Y")) ActionManager::redo();
            }

            if (VortexKeyboard::get_key_down("P")) VortexGUI::show_terminal = !VortexGUI::show_terminal;
            if (VortexKeyboard::get_key_down("X")) VortexGUI::show_camera_info = !VortexGUI::show_camera_info;
            if (VortexKeyboard::get_key_down("W")) VortexGUI::show_creator_window = !VortexGUI::show_creator_window;
            if (VortexKeyboard::get_key_down("E")) VortexGUI::show_engine_stats = !VortexGUI::show_engine_stats;
            if (VortexKeyboard::get_key_down("H")) VortexGUI::show_stack_history_window = !VortexGUI::show_stack_history_window;
            if (VortexKeyboard::get_key_down("A")) VortexGUI::show_asset_browser = !VortexGUI::show_asset_browser;
            if (VortexKeyboard::get_key_down("F")) VortexGUI::show_file_viewer = !VortexGUI::show_file_viewer;

            if (VortexKeyboard::get_key_down("C")) trigger_compile();
            if (VortexKeyboard::get_key_down("D")) duplicate_models();
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

    VortexAssetManager::clean_up();
    VortexObjectManager::clean_up();
    VortexAudio::clean_up();
    VortexDebugRenderer::get().clean_up();
    VortexGUI::clean_up();
    ActionManager::clear_stack_history();
    VortexProject::clean_playmode_backups();

    if (game_code)
    {
        game_code->unload();
        delete game_code;
        game_code = nullptr;
    }

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
    GLFWmonitor* monitor = get_current_monitor(window);
    if (!monitor) monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    if (is_fullscreen)
    {
        if (glfwGetWindowMonitor(window) == nullptr)
        {
            int x, y;
            glfwGetWindowPos(window, &x, &y);
            stored_window_width = static_cast<float>(x);
            stored_window_height = static_cast<float>(y);

            int left, top, right, bottom;
            glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

            stored_window_y_pos -= top;
            stored_window_x_pos -= left;
        }
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
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
    }

    if (is_fullscreen)
    {
        default_window_width = static_cast<float>(mode->width);
        default_window_height = static_cast<float>(mode->height);
    }
    else
    {
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
    init_game_code();

    VORTEX_INFO("VORTEX ENGINE RUNNING ON:");
    VORTEX_INFO("VENDOR:   ", glGetString(GL_VENDOR));
    VORTEX_INFO("RENDERER: ", glGetString(GL_RENDERER));

    const GLubyte *vendor_ptr = glGetString(GL_VENDOR);
    const GLubyte *renderer_ptr = glGetString(GL_RENDERER);

    VortexGUI::_vendor_ = vendor_ptr ? reinterpret_cast<const char*>(vendor_ptr) : "Unknown Vendor";
    VortexGUI::_renderer_  = renderer_ptr ? reinterpret_cast<const char*>(renderer_ptr) : "Unknown Renderer";

    glfwShowWindow(window);

    load_splash_screen();

    last_frame = static_cast<float>(glfwGetTime());

    while(!glfwWindowShouldClose(window))
    {
        check_for_hot_reload();

        if (game_code->is_valid && current_state != EngineState::PROJECT_HUB)
        {
            game_code->Update(&game_memory, this, deltaTime);
        }

        glfwPollEvents();
        check_key_press();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - last_frame;
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
            if (!show_mouse_cursor && current_state == EngineState::EDITOR &&
                (
                    (VortexGUI::is_scene_view_visible && VortexGUI::show_scene_viewport) ||
                    (VortexGUI::is_render_view_visible && VortexGUI::show_render_scene_viewport)
                )
            )
            {
                editor_camera->check_camera_movement(deltaTime);
            }

            if ((!VortexGUI::show_scene_viewport && !VortexGUI::show_render_scene_viewport) && !show_mouse_cursor)
            {
                show_mouse(true);
            }

            if (current_state == EngineState::PLAY)
            {
                VortexObjectManager::update(deltaTime);
            }

            if (VortexGUI::viewport_width > 0.0f && VortexGUI::viewport_height > 0.0f &&
            (VortexGUI::viewport_height != VortexGUI::scene_height || VortexGUI::viewport_width != VortexGUI::scene_width))
            {
                VortexGUI::resize_scene_fbo(
                    static_cast<int>(VortexGUI::viewport_width),
                    static_cast<int>(VortexGUI::viewport_height)
                );
            }

            if (VortexGUI::render_viewport_width > 0.0f && VortexGUI::render_viewport_height > 0.0f &&
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

            // Second Pass (Render View)
            if (VortexGUI::is_render_view_visible && VortexGUI::show_render_scene_viewport)
            {
                if (post_processor)
                {
                    post_processor->begin();
                }
                else
                {
                    VortexGUI::bind_render_framebuffer();
                }

                glViewport(0, 0, static_cast<GLsizei>(VortexGUI::render_scene_width), static_cast<GLsizei>(VortexGUI::render_scene_height));

                if (VortexGUI::render_scene_height > 0 && camera)
                {
                    render_camera->set_aspect_ratio(VortexGUI::render_scene_width / VortexGUI::render_scene_height);
                }

                if (!skybox) glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

            VortexGUI::draw_compiler_modal();
            VortexGUI::draw_exit_modal();

            VortexGUI::draw_editor_render_viewport(deltaTime, render_camera);
            VortexGUI::draw_editor_viewport(deltaTime, editor_camera);
        }

        if (is_playing_splash)
        {
            draw_splash_overlay(deltaTime);
        }

        VortexGUI::render();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        VortexKeyboard::update();
        VortexMouse::update();
        VortexAudio::update();
        VortexDebugRenderer::get().update();
        glfwSwapBuffers(window);
    }
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

void VortexApplication::init_game_code()
{
    game_memory = {};
    game_memory.is_initialized = false;
    game_memory.registry_context = &ScriptRegistry::get();

    #ifdef _WIN32
        std::string game_code_name = "VortexGame.dll";
    #else
        std::string game_code_name = "./libVortexGame.so";
    #endif

    game_code = new GameCode(game_code_name);
    game_code->load();

    if (game_code->is_valid && !game_memory.is_initialized)
    {
        game_code->Init(&game_memory, this);
    }
}

void VortexApplication::check_for_hot_reload()
{
    if (!game_code || !game_code->need_reload()) return;

    VORTEX_INFO("[ENGINE] Recompilation detected! Preparing hot reload...");
    std::error_code ec;

    #ifdef _WIN32
        std::string game_code_name = "VortexGame.dll";
        std::string test_temp_path = "VortexGame_test.dll";
    #else
        std::string game_code_name = "./libVortexGame.so";
        std::string test_temp_path = "./libVortexGame_test.so";
    #endif

    std::filesystem::copy_file(game_code_name, test_temp_path, std::filesystem::copy_options::overwrite_existing, ec);

    int retries = 0;
    while (retries < 10)
    {
        std::filesystem::copy_file(game_code_name, test_temp_path, std::filesystem::copy_options::overwrite_existing, ec);
        if (!ec) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        retries++;
    }

    if (ec)
    {
        VORTEX_WARN("[ENGINE] Compiler is still writing. Waiting...");
        return;
    }
    std::filesystem::remove(test_temp_path, ec);

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        for (VortexMonoBehaviour* script : model->behaviours)
        {
            delete script;
        }
        model->behaviours.clear();
    }

    ScriptRegistry::get().clear();

    game_code->unload();
    game_code->load();

    if (game_code->is_valid)
    {
        game_code->Init(&game_memory, this);

        for (VortexModel* model : VortexObjectManager::active_models)
        {
            std::vector<VortexMonoBehaviour*> restored_behaviours;
            std::vector<std::string> restored_names;

            for (const std::string& script_name : model->script_names)
            {
                VortexMonoBehaviour* fresh_script = ScriptRegistry::get().create(script_name);

                if (fresh_script)
                {
                    fresh_script->vortexGameObject = model;
                    fresh_script->vortexEngine = this;
                    fresh_script->vortexTransform = &model->transform;

                    restored_behaviours.push_back(fresh_script);
                    restored_names.push_back(script_name);
                }
                else
                {
                    VORTEX_WARN("[ENGINE] Script '", script_name, "' is missing from the DLL. Removing from model.");
                }
            }

            model->behaviours = restored_behaviours;
            model->script_names = restored_names;
        }
        VORTEX_INFO("[ENGINE] Hot Reload Complete! All pointers safely restored.");
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

    VortexGUI::show_inspector = false;
    VortexGUI::show_camera_info = false;
    VortexGUI::show_creator_window = false;
    VortexGUI::show_engine_stats = false;
    VortexGUI::show_terminal = false;
    VortexGUI::show_skybox_post_process_options = false;
    VortexGUI::show_scene_viewport = false;
    VortexGUI::show_stack_history_window = false;
    VortexGUI::show_asset_browser = false;
    VortexGUI::show_settings_window = false;
    VortexGUI::show_file_viewer = false;

    VortexGUI::show_render_scene_viewport = true;
    if (!VortexGUI::render_scene_fbo_initialized)
    {
        VortexGUI::setup_render_scene_fbo(VortexGUI::render_scene_width, VortexGUI::render_scene_height);
        VortexGUI::render_scene_fbo_initialized = true;
    }

    for (VortexModel* model : VortexGUI::m_selected_models)
    {
        if (model) model->is_selected = false;
    }
    VortexGUI::m_selected_models.clear();

    for (VortexModel *model : VortexObjectManager::active_models)
    {
        for (VortexMonoBehaviour *script : model->behaviours)
        {
            script->on_start();
        }
    }
}

void VortexApplication::exit_play_mode()
{
    VORTEX_INFO("[ENGINE] Exiting Play Mode...");

    current_state = EngineState::EDITOR;
    VortexGUI::show_inspector = true;
    VortexGUI::show_creator_window = true;
    VortexGUI::show_terminal = true;
    VortexGUI::show_skybox_post_process_options = true;
    VortexGUI::show_scene_viewport = true;
    VortexGUI::show_asset_browser = true;

    show_mouse(true);

    camera = editor_camera;

    VortexUIManager::cleanup();

    VortexObjectManager::clear_scene();
    VortexGUI::m_selected_models.clear();

    std::string project_name = VortexGUI::m_new_project_name;
    VortexProject::take_snapshot(SnapshotState::LOAD, this, "temp_playmode_backup_" + project_name);
}

void VortexApplication::trigger_compile()
{
    if (CompilerState::is_compiling.load()) return;

    CompilerState::progress.store(0.0f);
    CompilerState::is_compiling.store(true);

    {
        std::lock_guard<std::mutex> lock(CompilerState::status_mutex);
        CompilerState::status_text = "Starting CMake Build...";
    }

    std::thread([]()
    {
        #ifdef _WIN32
            #define POPEN _popen
            #define PCLOSE _pclose
        #else
            #define POPEN popen
            #define PCLOSE pclose
        #endif

        FILE* pipe = POPEN("make VortexGame 2>&1", "r");

        if (pipe)
        {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string line = buffer;
                if (!line.empty() && line.back() == '\n') line.pop_back();

                {
                    std::lock_guard<std::mutex> lock(CompilerState::status_mutex);
                    CompilerState::status_text = line;
                }

                size_t bracket_open = line.find('[');
                size_t percent_sign = line.find('%');

                if (bracket_open != std::string::npos && percent_sign != std::string::npos && percent_sign > bracket_open)
                {
                    std::string num_str = line.substr(bracket_open + 1, percent_sign - bracket_open - 1);
                    try {
                        float percent = std::stof(num_str) / 100.0f;
                        CompilerState::progress.store(percent);
                    } catch (...) {}
                }
            }
            PCLOSE(pipe);
        }

        CompilerState::progress.store(1.0f);

        {
            std::lock_guard<std::mutex> lock(CompilerState::status_mutex);
            CompilerState::status_text = "Build Complete! Triggering Hot Reload...";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        CompilerState::is_compiling.store(false);

    }).detach();
}

GLFWwindow* VortexApplication::get_window_ptr() const { return window; }
float VortexApplication::get_delta_time() const { return deltaTime; }
float VortexApplication::get_width() const { return default_window_width; }
float VortexApplication::get_height() const { return default_window_height; }

bool VortexApplication::is_mouse_visible() { return show_mouse_cursor; }

EngineState VortexApplication::get_state() const { return current_state; }
bool VortexApplication::is_wireframe_enabled() const { return show_wireframe; }
bool VortexApplication::is_exit_modal_open() const { return show_exit_modal; }
bool VortexApplication::has_unsaved() const { return has_unsaved_changes; }

VortexCamera *VortexApplication::get_camera() const { return camera; }
VortexCamera *VortexApplication::get_editor_camera() const { return editor_camera; }
// ShadowManager *VortexApplication::get_shadow_manager() const { return shadow_manager; }

void VortexApplication::set_camera(VortexCamera *camera) { this->camera = camera; }

void VortexApplication::set_state(EngineState state) { current_state = state; }
void VortexApplication::toggle_wireframe(bool enable) { show_wireframe = enable; }
void VortexApplication::toggle_exit_modal(bool show) { show_exit_modal = show; }

void VortexApplication::mark_unsaved_changes() { has_unsaved_changes = true; }
void VortexApplication::clear_unsaved_changes() {has_unsaved_changes = false; }
