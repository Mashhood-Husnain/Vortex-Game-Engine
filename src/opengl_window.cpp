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
        }
    }
}

OpenGLWindow::OpenGLWindow(std::string window_name, int width, int height)
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
    window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }

    stored_window_width = width;
    stored_window_height = height;
    glfwGetWindowPos(window, &stored_window_x_pos, &stored_window_y_pos);
}

OpenGLWindow::~OpenGLWindow()
{
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
        return;
    }

    glfwSetWindowMonitor(window, nullptr, stored_window_x_pos, stored_window_y_pos, stored_window_width, stored_window_height, 0);
    glfwFocusWindow(window);
}

void OpenGLWindow::run(std::function<void()> draw_callback)
{
    while(!glfwWindowShouldClose(window))
    {
        // rendering
        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_callback();
        
        // swap buffers and poll IO
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
