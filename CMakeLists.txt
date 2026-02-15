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

# Get all shader files
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/src/shaders/*")

foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

# Get all files within the models directory
file(GLOB_RECURSE MODEL_FILES "${CMAKE_SOURCE_DIR}/src/models/*")

foreach(MODEL ${MODEL_FILES})
    # 1. Get the path relative to the source 'models' folder
    # Example: src/models/mtl/textures/house.png -> mtl/textures/house.png
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/src/models" ${MODEL})

    # 2. Copy the file while preserving the relative directory structure
    configure_file(${MODEL} "${CMAKE_BINARY_DIR}/models/${REL_PATH}" COPYONLY)
endforeach()

# --- 3. Setup Your Game ---
add_executable(game 
    src/main.cpp
    src/opengl_window.cpp
    src/opengl_window.hpp
    src/opengl_shaders.cpp
    src/opengl_shaders.hpp
    src/opengl_camera.cpp
    src/opengl_camera.hpp
    src/opengl_objreader.cpp
    src/opengl_objreader.hpp
    src/stb_image.cpp
    src/stb_image.h
    src/player.cpp
    src/player.hpp
    # add other source files here
    # example: src/Game.cpp, src/Renderer.cpp, etc.
)

# --- 4. Link Everything ---
target_link_libraries(game glfw glad)
target_include_directories(game PUBLIC vendor/glm)

find_package(Threads REQUIRED)
target_link_libraries(game Threads::Threads ${CMAKE_DL_LIBS})
