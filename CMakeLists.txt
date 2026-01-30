cmake_minimum_required(VERSION 3.10)
project(OpenGLGame)

# C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. Setup GLFW (Windowing) ---
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# --- 2. Setup GLAD (OpenGL Loader) ---
add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# --- 3. Setup Your Game ---
add_executable(Game 
    src/main.cpp
    # add other source files here
    # example: src/Game.cpp, src/Renderer.cpp, etc.
)

# --- 4. Link Everything ---
target_link_libraries(Game glfw glad)
target_include_directories(Game PUBLIC vendor/glm)

find_package(Threads REQUIRED)
target_link_libraries(Game Threads::Threads ${CMAKE_DL_LIBS})
