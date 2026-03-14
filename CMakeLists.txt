cmake_minimum_required(VERSION 3.10)
project(VortexEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vendors
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

# GLAD
add_library(glad vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC vendor/glad/include)

# STB_IMAGE
add_library(stb_image vendor/stbimage/stb_image.cpp)
target_include_directories(stb_image PUBLIC vendor/stbimage)

# IMGUI
add_library(imgui
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_impl_glfw.cpp
    vendor/imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui)

# Shaders & Assets
file(GLOB SHADER_FILES "Vortex/shaders/*")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

file(GLOB_RECURSE MODEL_FILES "Vortex/assets/*")
foreach(MODEL ${MODEL_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/Vortex/assets" ${MODEL})
    configure_file(${MODEL} "${CMAKE_BINARY_DIR}/assets/${REL_PATH}" COPYONLY)
endforeach()

# EXECUTABLE
add_executable(engine 
    Vortex/world/world.cpp
    Vortex/src/vortex_window.cpp
    Vortex/src/vortex_shaders.cpp
    Vortex/src/vortex_camera.cpp
    Vortex/src/vortex_model.cpp
    Vortex/src/vortex_player.cpp
    Vortex/src/vortex_particlesystem.cpp
    Vortex/src/vortex_shadowmanager.cpp
    Vortex/src/vortex_gui.cpp
)

# INCLUDE DIRECTORIES
target_include_directories(engine PUBLIC 
    Vortex/include
    vendor/glm
    vendor/glad/include
    vendor/stbimage
    vendor/imgui
)

# LINKING
target_link_libraries(engine glfw glad stb_image imgui)

find_package(Threads REQUIRED)
target_link_libraries(engine Threads::Threads ${CMAKE_DL_LIBS})
