/*
 * File: vortex_application.cpp
 * Project: VortexEngine
 * Description: Implementation of window
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_application.hpp"

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

    if (app)
    {
        glViewport(0, 0, width, height);
        app->default_window_width = width;
        app->default_window_height = height;
        app->camera->aspect_ratio = static_cast<float>(width) / height;
    }
}

void VortexApplication::check_key_press()
{
    if (current_state == EngineState::EDITOR)
    {
        if (VortexKeyboard::get_key_down("F"))
        {
            is_fullscreen = !is_fullscreen;
            change_window_size();
        }

        if (VortexKeyboard::get_key_down("T")) show_wireframe = !show_wireframe;
        if (VortexKeyboard::get_key_down("V")) view_world_axis = !view_world_axis;
        if (VortexKeyboard::get_key_down("M")) show_mouse(!show_mouse_cursor);
    }

    if (VortexKeyboard::get_key_down("TAB") && current_state == EngineState::PLAY)
    {
        gui.show_debug_gui = !gui.show_debug_gui;
    }

    if (VortexKeyboard::get_key_down("ESCAPE"))
    {
        if (current_state == EngineState::PLAY)
        {
            std::cout << "[ENGINE] Exiting Play Mode..." << std::endl;
            current_state = EngineState::EDITOR;
            gui.show_gui = true;
            gui.show_debug_gui = true;

            show_mouse(true);

            camera = editor_camera;

            std::string project_name = "temp_playmode_backup";
            SaveScene_snapshot scene_snapshot = {
                project_name,
                VortexObjectManager::active_models,
                VortexObjectManager::active_particlesystems,
                gui.m_selected_skybox_idx,
                gui.m_selected_shader_idx,
                this
            };

            VortexProject::load_project(&scene_snapshot);
        }
        else
        {
            glfwSetWindowShouldClose(window, true);
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

    // position location
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // color location
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

VortexApplication::VortexApplication(std::string window_name, int width, int height)
{
    // Initialize GLFW;
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Configure GLFW (Version 3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    // Create Window
    default_window_width = width;
    default_window_height = height;
    stored_window_width = width;
    stored_window_height = height;
    stored_window_x_pos = 100;
    stored_window_y_pos = 100;
    this->window_name = window_name + " - " + GLOBAL::VORTEX_VERSION;

    editor_camera = new VortexCamera(glm::vec3(15.0f, 2.0f, 1.0f));
    editor_camera->look_at(glm::vec3(0.0f, 2.0f, 0.0f));
    camera = editor_camera;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(default_window_width, default_window_height, this->window_name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // initialize window
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    worldaxis_shader = new VortexShader("shaders/world_axis.vert", "shaders/world_axis.frag");
    setup_world_axis_buffers();

    shadow_manager = new ShadowManager();

    environment_grid = new VortexGrid();

    VortexObjectManager::init();
    VortexKeyboard::init(window);
    VortexMouse::init(window);
    VortexAudio::init();

    // V-sync
    glfwSwapInterval(1);

    // enable depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // enable culling of back faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    change_window_size();

    srand(static_cast<unsigned int>(time(0)));

    int start_width, start_height;
    glfwGetFramebufferSize(window, &start_width, &start_height);

    glViewport(0, 0, start_width, start_height);

    if (this->camera)
    {
        this->camera->aspect_ratio = static_cast<float>(start_width) / static_cast<float>(start_height);
    }

    gui.init(this);
}

VortexApplication::~VortexApplication()
{
    delete worldaxis_shader;
    delete shadow_manager;
    delete skybox;
    delete environment_grid;
    delete editor_camera;

    VortexAssetManager::clean_up();
    VortexObjectManager::clean_up();
    VortexAudio::clean_up();

    if (window)
    {
        glfwDestroyWindow(window);
    }

    if (post_processor)
    {
        delete post_processor;
    }

    worldaxis_shader = nullptr;
    shadow_manager = nullptr;
    window = nullptr;
    camera = nullptr;
    editor_camera = nullptr;
    post_processor = nullptr;
    skybox = nullptr;
    environment_grid = nullptr;

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();

    std::cout << "[ENGINE] Successfully Closed Application!" << std::endl;
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

    if (post_processor)
    {
        post_processor->resize(default_window_width, default_window_height);
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
    std::cout << "VORTEX ENGINE RUNNING ON:" << std::endl;
    std::cout << "VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "RENDERER: " << glGetString(GL_RENDERER) << std::endl;

    glfwShowWindow(window);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        check_key_press();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - last_frame;
        last_frame = currentFrame;

        if (!show_mouse_cursor && current_state == EngineState::EDITOR)
        {
            editor_camera->check_camera_movement(deltaTime);
        }

        // draw shadow map
        // shadow_manager->draw_shadow_map(draw_callback, editor_camera);
        
        if (post_processor)
        {
            post_processor->begin();
            glViewport(0, 0, default_window_width, default_window_height);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, default_window_width, default_window_height);
        }
        
        // rendering
        if (!skybox)
        {
            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gui.update();
        gui.engine_stats();
        gui.camera_info(camera);
        gui.post_process_options();
        gui.skybox_options();
        gui.creator_window();
        gui.begin_scene_inspector();

        if (skybox)
        {
            skybox->draw(camera);
        }

        if (current_state == EngineState::PLAY)
        {
            VortexObjectManager::update(deltaTime);
        }

        draw_callback();

        VortexObjectManager::draw(*camera, show_wireframe);
        VortexObjectManager::check_object_status();

        gui.end_scene_inspector();

        environment_grid->draw(*camera);

        if (post_processor) {
            post_processor->end();

            glViewport(0, 0, default_window_width, default_window_height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            post_processor->draw(currentFrame);
        }
        
        if (view_world_axis)
        {
            draw_world_axis();
            draw_world_axis_gizmo();
        }

        gui.render();
        VortexKeyboard::update();
        VortexMouse::update();
        VortexAudio::update();
        // swap buffers and poll IO
        glfwSwapBuffers(window);
    }
}

GLFWwindow* VortexApplication::get_window_ptr()
{
    return window;
}

void VortexApplication::set_post_processor(PostProcessor *post_processor)
{
    if (this->post_processor)
    {
        delete this->post_processor;
    }

    this->post_processor = post_processor;

    if (this->post_processor)
    {
        this->post_processor->resize(default_window_width, default_window_height);
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

    if (status)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}
