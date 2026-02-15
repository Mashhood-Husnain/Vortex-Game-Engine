#include "opengl_window.hpp"

GLFWmonitor* OpenGLWindow::get_current_monitor(GLFWwindow* window)
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

void OpenGLWindow::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto* app = static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

    if (app)
    {
        glViewport(0, 0, width, height);
        app->default_window_width = width;
        app->default_window_height = height;
        app->camera->aspect_ratio = static_cast<float>(width) / height;
    }
}

void OpenGLWindow::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* app = static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

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
            case GLFW_KEY_R:
                app->camera->anchored = !app->camera->anchored;
                break;
        }
    }
}

OpenGLWindow::OpenGLWindow(std::string window_name, OpenGLCamera* camera, int width, int height)
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
}

OpenGLWindow::~OpenGLWindow()
{
    window = nullptr;
    camera = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();
}

void OpenGLWindow::set_fullscreen()
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

void OpenGLWindow::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    auto* app = static_cast<OpenGLWindow*>(glfwGetWindowUserPointer(window));

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

void OpenGLWindow::run(std::function<void()> draw_callback)
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

        // std::cout << "mouse X: "<< mouse_last_x << "mouse Y: "<< mouse_last_y << std::endl;
        // std::cout << camera->aspect_ratio << std::endl;

        draw_callback();
        
        // swap buffers and poll IO
        glfwSwapBuffers(window);
    }
}

GLFWwindow* OpenGLWindow::get_window_ptr()
{
    return window;
}
