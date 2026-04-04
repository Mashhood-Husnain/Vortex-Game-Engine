/*
 * File: vortex_window.cpp
 * Project: VortexEngine
 * Description: Implementation of window
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_window.hpp"

GLFWmonitor* VortexWindow::get_current_monitor(GLFWwindow* window)
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

void VortexWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<VortexWindow*>(glfwGetWindowUserPointer(window));

    if (app)
    {
        glViewport(0, 0, width, height);
        app->default_window_width = width;
        app->default_window_height = height;
        app->camera->aspect_ratio = static_cast<float>(width) / height;
    }
}

void VortexWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* app = static_cast<VortexWindow*>(glfwGetWindowUserPointer(window));

    if (!app) return;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_F:
                app->is_fullscreen = !app->is_fullscreen;
                app->change_window_size();
                break;
            case GLFW_KEY_T:
                app->show_wireframe = !app->show_wireframe;
                break;
            // temporary solution
            // disabled for now, until i clean up the player code and come up with a better solution
            case GLFW_KEY_R:
                app->camera->anchored = !app->camera->anchored;
                break;
            case GLFW_KEY_V:
                app->view_world_axis = !app->view_world_axis;
                break;
            case GLFW_KEY_M:
                app->show_mouse_cursor = !app->show_mouse_cursor;

                if (app->show_mouse_cursor) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

                break;
        }
    }
}

void VortexWindow::setup_world_axis_buffers()
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

void VortexWindow::draw_world_axis()
{
    worldaxis_shader->use();

    worldaxis_shader->setMat4("view", camera->getViewMatrix());
    worldaxis_shader->setMat4("projection", camera->getProjectionMatrix());

    glLineWidth(2.0f);
    glBindVertexArray(world_axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void VortexWindow::draw_world_axis_gizmo()
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

VortexWindow::VortexWindow(std::string window_name, VortexCamera* camera, int width, int height)
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

    // Create Window
    default_window_width = width;
    default_window_height = height;
    this->stored_window_width = width;
    this->stored_window_height = height;
    this->stored_window_x_pos = 100;
    this->stored_window_y_pos = 100;
    this->window_name = window_name + " - " + GLOBAL::VORTEX_VERSION;
    this->camera = camera;

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
    glfwSetKeyCallback(window, key_callback);
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

    particle_shader = new VortexShader("shaders/particles.vert", "shaders/particles.frag");
    model_shader = new VortexShader("shaders/default.vert", "shaders/default.frag");

    environment_grid = new VortexGrid();

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
    gui.init(window);

    srand(static_cast<unsigned int>(time(0)));

    int start_width, start_height;
    glfwGetFramebufferSize(window, &start_width, &start_height);

    glViewport(0, 0, start_width, start_height);

    if (this->camera)
    {
        this->camera->aspect_ratio = static_cast<float>(start_width) / static_cast<float>(start_height);
    }
}

VortexWindow::~VortexWindow()
{
    delete worldaxis_shader;
    delete shadow_manager;
    delete skybox;
    delete particle_shader;
    delete model_shader;
    delete environment_grid;

    for (VortexModel* model : dynamic_models)
    {
        delete model;
    }
    dynamic_models.clear();

    for (ParticleSystem* ps : dynamic_particlesystems)
    {
        delete ps;
    }
    dynamic_particlesystems.clear();

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
    post_processor = nullptr;
    skybox = nullptr;
    particle_shader = nullptr;
    model_shader = nullptr;
    environment_grid = nullptr;

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void VortexWindow::change_window_size()
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

void VortexWindow::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    auto* app = static_cast<VortexWindow*>(glfwGetWindowUserPointer(window));

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
            app->camera->processMouseMovement(xoffset, yoffset);
        }
    }
}

void VortexWindow::run(std::function<void()> draw_callback)
{
    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "VORTEX ENGINE RUNNING ON:" << std::endl;
    std::cout << "VENDOR:   " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "-------------------------------------------------------" << std::endl;

    glfwShowWindow(window);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - last_frame;
        last_frame = currentFrame;

        if (!show_mouse_cursor)
        {
            camera->check_camera_movement(window, deltaTime);
        }

        // draw shadow map
        // shadow_manager->draw_shadow_map(draw_callback, camera);
        
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
        gui.show_engine_stats();
        gui.show_camera_info(camera);
        gui.show_post_process_options(this);
        gui.show_skybox_options(this);
        gui.show_creator_window(this, dynamic_particlesystems, dynamic_models);
        gui.begin_scene_inspector();

        if (skybox)
        {
            skybox->draw(camera);
        }

        draw_callback();

        for (VortexModel *model : dynamic_models)
        {
            model->draw(*model_shader, *camera, show_wireframe);
        }

        for (ParticleSystem *ps : dynamic_particlesystems)
        {
            ps->update(deltaTime);
            ps->draw(*particle_shader, *camera);
        }

        environment_grid->draw(*camera);

        dynamic_particlesystems.erase(
            std::remove_if(
                dynamic_particlesystems.begin(),
                dynamic_particlesystems.end(),
                [](ParticleSystem *ps){
                    if (ps->should_destroy)
                    {
                        delete ps;
                        return true;
                    }
                    return false;
                }),
            dynamic_particlesystems.end()
        );

        dynamic_models.erase(
            std::remove_if(
                dynamic_models.begin(),
                dynamic_models.end(),
                [](VortexModel *model){
                    if (model->should_destroy)
                    {
                        delete model;
                        return true;
                    }
                    return false;
                }),
            dynamic_models.end()
        );

        gui.end_scene_inspector();

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
        // swap buffers and poll IO
        glfwSwapBuffers(window);
    }
}

GLFWwindow* VortexWindow::get_window_ptr()
{
    return window;
}

void VortexWindow::set_post_processor(PostProcessor *post_processor)
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

void VortexWindow::set_skybox(VortexSkybox *skybox)
{
    delete this->skybox;
    this->skybox = skybox;
}
