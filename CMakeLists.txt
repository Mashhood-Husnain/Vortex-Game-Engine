cmake_minimum_required(VERSION 3.10)
project(OpenGLGame)

# Use C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Setup GLFW (Windowing) ---
# This makes GLFW build as part of your project
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# --- 2. Setup GLAD (OpenGL Loader) ---
# We build GLAD as a small static library
add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# --- 3. Setup Your Game ---
add_executable(Game 
    src/main.cpp
    # add other source files here
    # example: src/Game.cpp, src/Renderer.cpp, etc.
)

# --- 4. Link Everything ---
# Link your game to GLFW, GLAD, and GLM
target_link_libraries(Game glfw glad)
target_include_directories(Game PUBLIC vendor/glm)

# Note: On Linux, GLFW needs system threads and X11/Wayland libraries
# CMake usually handles this, but sometimes you need to explicitly link 'dl'
find_package(Threads REQUIRED)
target_link_libraries(Game Threads::Threads ${CMAKE_DL_LIBS})
