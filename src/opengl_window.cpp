#include "opengl_window.hpp"

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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
}

OpenGLWindow::~OpenGLWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void OpenGLWindow::check_input()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void OpenGLWindow::run()
{
    while(!glfwWindowShouldClose(window))
    {
        // Check for any user inputs
        check_input();

        // rendering
        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // swap buffers and poll IO
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
