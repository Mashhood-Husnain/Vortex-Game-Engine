cmake_minimum_required(VERSION 3.10)
project(VortexEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----------------------------
# OS-Specific Definitions
# ----------------------------
if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES NOMINMAX WIN32_LEAN_AND_MEAN)
    add_compile_options(/wd4005)
endif()

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(VORTEX_LINUX_DEPS x11 xrandr xinerama xcursor xi xkbcommon)

    if(NOT VORTEX_LINUX_DEPS_FOUND)
        message(FATAL_ERROR "\n[VORTEX ENGINE] Missing required Linux graphics headers!\n"
                            "Please run the included setup script: ./setup_linux.sh\n"
                            "Or manually install: X11, Xrandr, Xinerama, Xcursor, Xi, and Xkbcommon dev packages.\n")
    endif()
endif()

# ----------------------------
# Vendors
# ----------------------------
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
target_include_directories(imgui PUBLIC vendor/imgui vendor/glfw/include)

# ImGuizmo
add_library(ImGuizmo vendor/ImGuizmo/ImGuizmo.cpp)
target_include_directories(ImGuizmo PUBLIC vendor/ImGuizmo vendor/imgui)

# NVML
target_include_directories(imgui PUBLIC vendor/nvml)
include_directories(vendor/nvml)

# miniaudio
add_library(miniaudio vendor/miniaudio/miniaudio.c)
target_include_directories(miniaudio PUBLIC vendor/miniaudio)

# Environment Variables (.env)
# ----------------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
    configure_file("${CMAKE_SOURCE_DIR}/.env" "${CMAKE_BINARY_DIR}/.env" COPYONLY)
endif()

# ----------------------------
# Shaders & Assets
# ----------------------------
file(GLOB SHADER_FILES CONFIGURE_DEPENDS "Vortex/shaders/*")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME} COPYONLY)
endforeach()

file(GLOB_RECURSE MODEL_FILES CONFIGURE_DEPENDS "Vortex/assets/*")
foreach(MODEL ${MODEL_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}/Vortex/assets" ${MODEL})
    configure_file(${MODEL} "${CMAKE_BINARY_DIR}/assets/${REL_PATH}" COPYONLY)
endforeach()

# ----------------------------
# EXECUTABLE
# ----------------------------
add_executable(engine
    Vortex/world/world.cpp

    Vortex/src/vortex_application.cpp
    Vortex/src/vortex_shaders.cpp
    Vortex/src/vortex_camera.cpp
    Vortex/src/vortex_model.cpp
    Vortex/src/vortex_particlesystem.cpp
    Vortex/src/vortex_shadowmanager.cpp
    Vortex/src/vortex_gui_core.cpp
    Vortex/src/vortex_gui_creator.cpp
    Vortex/src/vortex_gui_environment.cpp
    Vortex/src/vortex_gui_inspector.cpp
    Vortex/src/vortex_gui_overlays.cpp
    Vortex/src/vortex_gui_editor.cpp
    Vortex/src/vortex_gui_dockspace.cpp
    Vortex/src/vortex_gui_terminal.cpp
    Vortex/src/vortex_gui_menus.cpp
    Vortex/src/vortex_gui_assetbrowser.cpp
    Vortex/src/vortex_gui_settings.cpp
    Vortex/src/vortex_postprocessor.cpp
    Vortex/src/vortex_skybox.cpp
    Vortex/src/vortex_grid.cpp
    Vortex/src/vortex_assetmanager.cpp
    Vortex/src/vortex_keyboard.cpp
    Vortex/src/vortex_mouse.cpp
    Vortex/src/vortex_physics.cpp
    Vortex/src/vortex_collider.cpp
    Vortex/src/vortex_objectmanager.cpp
    Vortex/src/vortex_hud.cpp
    Vortex/src/vortex_audio.cpp
    Vortex/src/vortex_rigidbody.cpp
    Vortex/src/vortex_uimanager.cpp
    Vortex/src/vortex_canvas.cpp
    Vortex/src/vortex_debugrenderer.cpp
    Vortex/src/vortex_light.cpp
    Vortex/src/vortex_actions.cpp

    Vortex/src/util/vortex_save_load.cpp
    Vortex/src/util/vortex_save_load_encryption.cpp
    Vortex/src/util/vortex_nvml.cpp
)

# ----------------------------
# Get user's custom scripts
# ----------------------------
file(GLOB_RECURSE USER_SCRIPTS CONFIGURE_DEPENDS "Vortex/Game/Scripts/*.cpp")

set_target_properties(engine PROPERTIES ENABLE_EXPORTS TRUE)

add_library(VortexGame SHARED
    Vortex/src/VortexEngine.cpp
    ${USER_SCRIPTS}
)

target_link_libraries(VortexGame engine)

set_target_properties(VortexGame PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ----------------------------
# INCLUDE DIRECTORIES
# ----------------------------
target_include_directories(engine PUBLIC
    Vortex/include
    Vortex/include/util
    vendor/glm
    vendor/glad/include
    vendor/stbimage
    vendor/imgui
    vendor/nvml
    vendor/nlohmann_json
    vendor/miniaudio
    vendor/ImGuizmo
)

# ----------------------------
# LINKING
# ----------------------------
find_package(Threads REQUIRED)

target_link_libraries(engine
    glfw
    glad
    stb_image
    imgui
    miniaudio
    ImGuizmo
    Threads::Threads
)

# OS-Specific Libraries
if (UNIX AND NOT APPLE)
    target_link_libraries(engine
        m
        rt
        asound
        dl
        ${CMAKE_DL_LIBS}
    )
endif()
