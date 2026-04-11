cmake_minimum_required(VERSION 3.10)
project(VortexEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# OS-Specific Definitions
# ----------------------------
if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES NOMINMAX WIN32_LEAN_AND_MEAN)
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

# NVML
target_include_directories(imgui PUBLIC vendor/nvml)
include_directories(vendor/nvml)

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
# Get user's custom scripts
# ----------------------------
file(GLOB_RECURSE USER_SCRIPTS CONFIGURE_DEPENDS "Vortex/Game/Scripts/*.cpp")

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
    Vortex/src/vortex_gui.cpp
    Vortex/src/vortex_postprocessor.cpp
    Vortex/src/util/vortex_nvml.cpp
    Vortex/src/vortex_skybox.cpp
    Vortex/src/vortex_grid.cpp
    Vortex/src/util/vortex_save_load.cpp
    Vortex/src/vortex_assetmanager.cpp
    ${USER_SCRIPTS}
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
    Threads::Threads
)

# OS-Specific Libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(engine ${CMAKE_DL_LIBS} dl)
endif()
