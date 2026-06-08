/*
 * File: world.cpp
 * Project: VortexEngine
 * Description: Running the engine
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_application.hpp"
#include "util/vortex_logs.hpp"

#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    extern "C" {
        __declspec(dllexport) __declspec(selectany) DWORD NvOptimusEnablement = 0x00000001;
        __declspec(dllexport) __declspec(selectany) int AmdPowerXpressRequestHighPerformance = 1;
    }
#else
    #include <unistd.h>
    extern char **environ;
#endif

int main(int argc, char **argv)
{
    #ifdef __linux__
        const char* nv_offload = std::getenv("__NV_PRIME_RENDER_OFFLOAD");

        bool needs_restart = false;
        if (!nv_offload || std::string(nv_offload) != "1") needs_restart = true;

        if (needs_restart)
        {
            setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
            setenv("__GL_SYNC_TO_VBLANK", "1", 1);
            setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
            setenv("__EGL_VENDOR_LIBRARY_NAME", "nvidia", 1);
            setenv("__VK_LAYER_NV_optimus", "NVIDIA_only", 1);

            unsetenv("WAYLAND_DISPLAY");
            setenv("XDG_SESSION_TYPE", "x11", 1);

            char exe_path[1024];
            ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
            if (len != -1)
            {
                exe_path[len] = '\0';
                execve(exe_path, argv, environ);
            }

            std::cerr << "[VORTEX ERROR] CRITICAL: Failed to restart process for GPU offload!" << std::endl;
            return EXIT_FAILURE;
        }
    #endif

    try
    {
        #if defined(_WIN32) || defined(_WIN64)
            char path[MAX_PATH];
            GetModuleFileNameA(NULL, path, MAX_PATH);
            std::filesystem::current_path(std::filesystem::path(path).parent_path());
        #else
            std::filesystem::path exe_path = std::filesystem::read_symlink("/proc/self/exe");
            std::filesystem::current_path(exe_path.parent_path());
        #endif

        VORTEX_INFO("[SYSTEM] Working Dirtectory set to: ", std::filesystem::current_path());
    }
    catch(const std::exception &e)
    {
        VORTEX_ERROR("[SYSTEM] Failed to set working directory: ", e.what());
    }

    VortexApplication VortexEngineApplication("Vortex Engine");

    VortexEngineApplication.run([&](){});

    return 0;
}
