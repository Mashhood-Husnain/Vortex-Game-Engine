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
                app->set_fullscreen();
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
        }
    }
}

void VortexWindow::setup_world_axis_buffers()
{
    glGenVertexArrays(1, &world_axisVAO);
    glGenBuffers(1, &world_axisVBO);
    glBindVertexArray(world_axisVAO);
    glBindBuffer(GL_ARRAY_BUFFER, world_axisVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(global_WORLDAXESVERTICES), global_WORLDAXESVERTICES, GL_STATIC_DRAW);

    // position location
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // color location
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
}

void VortexWindow::draw_world_axis()
{
    glUseProgram(worldaxis_shader->shader_program);

    glUniformMatrix4fv(glGetUniformLocation(worldaxis_shader->shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera->getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(worldaxis_shader->shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(camera->getProjectionMatrix()));

    glLineWidth(2.0f);
    glBindVertexArray(world_axisVAO);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void VortexWindow::draw_world_axis_gizmo()
{
    glDisable(GL_DEPTH_TEST);

    glUseProgram(worldaxis_shader->shader_program);

    glm::mat4 viewRotation = glm::mat4(glm::mat3(camera->getViewMatrix()));
    glm::mat4 orthoProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -1.0f, 10.0f);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glViewport(10, 10, 100, 100);

    glUniformMatrix4fv(glGetUniformLocation(worldaxis_shader->shader_program, "view"), 1, GL_FALSE, glm::value_ptr(viewRotation));
    glUniformMatrix4fv(glGetUniformLocation(worldaxis_shader->shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj));

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
    this->window_name = window_name;
    this->camera = camera;
    camera->aspect_ratio =static_cast<float>(default_window_width) / static_cast<float>(default_window_height);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
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

    // V-sync
    glfwSwapInterval(1);

    // enable depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // enable culling of back faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    stored_window_width = width;
    stored_window_height = height;
    glfwGetWindowPos(window, &stored_window_x_pos, &stored_window_y_pos);

    srand(static_cast<unsigned int>(time(0)));
}

VortexWindow::~VortexWindow()
{
    delete worldaxis_shader;

    worldaxis_shader = nullptr;
    window = nullptr;
    camera = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VortexWindow::set_fullscreen()
{
    if (is_fullscreen)
    {
        glfwGetWindowPos(window, &stored_window_x_pos, &stored_window_y_pos);
        glfwGetWindowSize(window, &stored_window_width, &stored_window_height);

        GLFWmonitor* monitor = get_current_monitor(window);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glfwFocusWindow(window);
        first_mouse = true;
        return;
    }

    glfwSetWindowMonitor(window, nullptr, stored_window_x_pos, stored_window_y_pos, stored_window_width, stored_window_height, 0);
    first_mouse = true;
    glfwFocusWindow(window);
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

        app->camera->processMouseMovement(xoffset, yoffset);
    }
}

void VortexWindow::run(std::function<void()> draw_callback)
{
    glfwShowWindow(window);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera->check_camera_movement(window, deltaTime);
        
        // rendering
        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        draw_callback();
        
        if (view_world_axis)
        {
            draw_world_axis();
            draw_world_axis_gizmo();
        }
        
        // swap buffers and poll IO
        glfwSwapBuffers(window);
    }
}

GLFWwindow* VortexWindow::get_window_ptr()
{
    return window;
}
