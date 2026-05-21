/*
 * File: vortex_application.cpp
 * Project: VortexEngine
 * Description: Implementation of window
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_application.hpp"

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
        app->default_window_width = width;
        app->default_window_height = height;
    }
}

void VortexApplication::request_exit()
{
    std::string project_name = std::string(gui.save_project_name);
    Snapshot snapshot = {
        project_name,
        VortexObjectManager::active_models,
        VortexObjectManager::active_particlesystems,
        gui.m_selected_skybox_idx,
        gui.m_selected_shader_idx,
        gui.explicit_empty_folders,
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

    if (current_state == EngineState::EDITOR)
    {
        if (VortexKeyboard::get_key_down("F"))
        {
            is_fullscreen = !is_fullscreen;
            change_window_size();
        }

        if (VortexKeyboard::get_key_down("Z")) show_wireframe = !show_wireframe;
        if (VortexKeyboard::get_key_down("V")) view_world_axis = !view_world_axis;
        if (VortexKeyboard::get_key_down("M")) show_mouse(!show_mouse_cursor);

        if (VortexKeyboard::get_key("LEFTCONTROL") && VortexKeyboard::get_key_down("P"))
        {
            gui.show_terminal = !gui.show_terminal;
        }

        if (VortexKeyboard::get_key("LEFTCONTROL") && VortexKeyboard::get_key_down("D"))
        {
            std::vector<VortexModel*> newly_cloned_models;

            for (VortexModel *current_model : gui.m_selected_models)
            {
                VortexModel *cloned_model = current_model->clone();
                VortexObjectManager::active_models.push_back(cloned_model);

                newly_cloned_models.push_back(cloned_model);

                current_model->is_selected = false;
            }

            gui.m_selected_models.clear();

            for (VortexModel * clone : newly_cloned_models)
            {
                clone->is_selected = true;
                gui.m_selected_models.insert(clone);
            }
        }

        if (VortexKeyboard::get_key("LEFTCONTROL") && VortexKeyboard::get_key_down("S"))
        {
            std::string project_name = std::string(gui.save_project_name);
            VortexProject::take_snapshot(SnapshotState::SAVE, this, project_name);
        }

        if (!gui.m_selected_models.empty(), VortexKeyboard::get_key_down("DELETE"))
        {
            for (VortexModel *model : gui.m_selected_models)
            {
                model->should_destroy = true;
                model->is_selected = false;
            }

            gui.m_selected_models.clear();
        }
    }

    if (VortexKeyboard::get_key_down("TAB") && current_state == EngineState::PLAY)
    {
        gui.show_debug_gui = !gui.show_debug_gui;
    }

    if (VortexKeyboard::get_key_down("ESCAPE"))
    {
        if (current_state == EngineState::PLAY)
        {
            VORTEX_INFO("[ENGINE] Exiting Play Mode...");

            current_state = EngineState::EDITOR;
            gui.show_gui = true;
            gui.show_debug_gui = true;
            gui.show_terminal = true;

            show_mouse(true);

            camera = editor_camera;

            VortexUIManager::cleanup();

            VortexProject::take_snapshot(SnapshotState::LOAD, this, "temp_playmode_backup");
        }
        else
        {
            request_exit();
        }
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

void VortexApplication::draw_world_axis()
{
    worldaxis_shader->use();

    worldaxis_shader->setMat4("view", camera->getViewMatrix());
    worldaxis_shader->setMat4("projection", camera->getProjectionMatrix());

    glLineWidth(2.0f);
    glBindVertexArray(world_axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void VortexApplication::draw_world_axis_gizmo()
{
    glDisable(GL_DEPTH_TEST);

    worldaxis_shader->use();

    glm::mat4 viewRotation = glm::mat4(glm::mat3(camera->getViewMatrix()));
    glm::mat4 orthoProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 10.0f);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    glViewport(10, 10, 100, 100);

    worldaxis_shader->setMat4("view", camera->getViewMatrix());
    worldaxis_shader->setMat4("projection", camera->getProjectionMatrix());

    glLineWidth(3.0f);
    glBindVertexArray(world_axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glEnable(GL_DEPTH_TEST);
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

    default_window_width = work_width;
    default_window_height = work_height;
    
    stored_window_width = default_window_width;
    stored_window_height = default_window_height;
    
    stored_window_x_pos = work_x;
    stored_window_y_pos = work_y;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    this->window_name = window_name + " - " + GLOBAL::VORTEX_VERSION;

    editor_camera = new VortexCamera(glm::vec3(15.0f, 2.0f, 1.0f));
    editor_camera->look_at(glm::vec3(0.0f, 2.0f, 0.0f));
    camera = editor_camera;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    window = glfwCreateWindow(default_window_width, default_window_height, this->window_name.c_str(), nullptr, nullptr);
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        VORTEX_ERROR("Failed to initialize GLAD");
        exit(EXIT_FAILURE);
    }

    int start_width, start_height;
    glfwGetFramebufferSize(window, &start_width, &start_height);

    worldaxis_shader = new VortexShader("shaders/world_axis.vert", "shaders/world_axis.frag");
    setup_world_axis_buffers();

    shadow_manager = new ShadowManager();
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

    gui.init(this, start_width, start_height);
}

VortexApplication::~VortexApplication()
{
    delete worldaxis_shader;
    delete shadow_manager;
    delete skybox;
    delete environment_grid;
    delete editor_camera;

    if (post_processor) delete post_processor;

    VortexAssetManager::clean_up();
    VortexObjectManager::clean_up();
    VortexAudio::clean_up();
    VortexDebugRenderer::get().clean_up();

    if (game_code)
    {
        game_code->unload();
        delete game_code;
        game_code = nullptr;
    }

    worldaxis_shader = nullptr;
    shadow_manager = nullptr;
    window = nullptr;
    camera = nullptr;
    editor_camera = nullptr;
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
            glfwGetWindowPos(window, &stored_window_x_pos, &stored_window_y_pos);
            int left, top, right, bottom;
            glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

            stored_window_y_pos -= top; 
            stored_window_x_pos -= left;
        }
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        glfwSetWindowMonitor(window, nullptr, stored_window_x_pos, stored_window_y_pos, stored_window_width, stored_window_height, 0);
    }

    if (is_fullscreen)
    {
        default_window_width  = mode->width;
        default_window_height = mode->height;
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

    gui._vendor_ = vendor_ptr ? reinterpret_cast<const char*>(vendor_ptr) : "Unknown Vendor";
    gui._renderer_  = renderer_ptr ? reinterpret_cast<const char*>(renderer_ptr) : "Unknown Renderer";

    glfwShowWindow(window);

    load_splash_screen();

    last_frame = static_cast<float>(glfwGetTime());

    while(!glfwWindowShouldClose(window))
    {
        check_for_hot_reload();

        if (game_code->is_valid)
        {
            game_code->Update(&game_memory, this, deltaTime);
        }

        glfwPollEvents();
        check_key_press();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - last_frame;
        last_frame = currentFrame;

        gui.update();

        if (!show_mouse_cursor && current_state == EngineState::EDITOR)
        {
            editor_camera->check_camera_movement(deltaTime);
        }

        if (gui.scene_height > 0) 
        {
            float target_aspect = static_cast<float>(gui.scene_width) / static_cast<float>(gui.scene_height);
            if (camera) camera->set_aspect_ratio(target_aspect);
            if (editor_camera) editor_camera->set_aspect_ratio(target_aspect);
        }

        if (gui.viewport_width > 0 && gui.viewport_height > 0 && 
           (gui.viewport_height != gui.scene_height || gui.viewport_width != gui.scene_width))
        {
            gui.resize_scene_fbo(gui.viewport_width, gui.viewport_height);
            if (post_processor) post_processor->resize(gui.scene_width, gui.scene_height);
        }

        if (post_processor)
        {
            post_processor->begin();
        }
        else
        {
            gui.bind_framebuffer();
        }

        glViewport(0, 0, gui.scene_width, gui.scene_height);
        
        if (!skybox) glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (skybox) skybox->draw(camera);

        if (current_state == EngineState::PLAY) VortexObjectManager::update(deltaTime);

        draw_callback();

        VortexObjectManager::draw(*camera, show_wireframe);
        VortexObjectManager::check_object_status();

        VortexDebugRenderer::get().render(camera);

        if (current_state == EngineState::EDITOR)
        {
            environment_grid->draw(*camera);

            if (view_world_axis)
            {
                draw_world_axis();
                draw_world_axis_gizmo();
            }
        }

        if (post_processor)
        {
            post_processor->end();
            gui.bind_framebuffer();
            glViewport(0, 0, gui.scene_width, gui.scene_height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            post_processor->draw(currentFrame);
        }

        VortexUIManager::render_ui();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, default_window_width, default_window_height);
        
        glClearColor(0.06f, 0.06f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        
        gui.build_dockspace();

        gui.engine_stats();
        gui.camera_info(camera);
        gui.post_process_options();
        gui.skybox_options();
        gui.creator_window();
        gui.begin_scene_inspector();
        gui.end_scene_inspector();
        gui.draw_exit_modal();
        gui.draw_terminal();
        gui.draw_compiler_modal();
        
        gui.draw_editor_viewport(deltaTime, camera);

        if (is_playing_splash)
        {
            draw_splash_overlay(deltaTime);
        }

        gui.render();

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
    if (this->post_processor) this->post_processor->resize(gui.scene_width, gui.scene_height);
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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
    }
    else
    {
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

GLFWwindow* VortexApplication::get_window_ptr() const { return window; }
float VortexApplication::get_delta_time() const { return deltaTime; }
int VortexApplication::get_width() const { return default_window_width; }
int VortexApplication::get_height() const { return default_window_height; }

EngineState VortexApplication::get_state() const { return current_state; }
bool VortexApplication::is_wireframe_enabled() const { return show_wireframe; }
bool VortexApplication::is_world_axis_visible() const { return view_world_axis; }
bool VortexApplication::is_exit_modal_open() const { return show_exit_modal; }
bool VortexApplication::has_unsaved() const { return has_unsaved_changes; }

VortexCamera *VortexApplication::get_camera() const { return camera; }
VortexCamera *VortexApplication::get_editor_camera() const { return editor_camera; }
ShadowManager *VortexApplication::get_shadow_manager() const { return shadow_manager; }

VortexGUI &VortexApplication::get_gui() { return gui; }

void VortexApplication::set_camera(VortexCamera *camera) { this->camera = camera; }

void VortexApplication::set_state(EngineState state) { current_state = state; }
void VortexApplication::toggle_wireframe(bool enable) { show_wireframe = enable; }
void VortexApplication::toggle_world_axis(bool enable) { view_world_axis = enable; }
void VortexApplication::toggle_exit_modal(bool show) { show_exit_modal = show; }

void VortexApplication::mark_unsaved_changes() { has_unsaved_changes = true; }
void VortexApplication::clear_unsaved_changes() {has_unsaved_changes = false; }
