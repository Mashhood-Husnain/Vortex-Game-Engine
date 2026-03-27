/*
 * File: vortex_gpu_pre_init.hpp
 * Project: VortexEngine
 * Description: Cross-platform GPU detection and offloading
 * Author: Mashhood Husnain
 * License: MIT
 */

#ifndef VORTEX_GPU_PREINIT_H
#define VORTEX_GPU_PREINIT_H

// WINDOWS: Request High-Performance GPU
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

extern "C" {
    __declspec(dllexport) __declspec(selectany) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) __declspec(selectany) int AmdPowerXpressRequestHighPerformance = 1;
}

// LINUX: Request High-Performance GPU
#elif defined(__linux__)
#include <cstdlib>
#include <unistd.h>
#include <string>

__attribute__((constructor))
static void initialize_vortex_hardware()
{
    const char* offload = std::getenv("__NV_PRIME_RENDER_OFFLOAD");
    const char* attempted = std::getenv("VORTEX_GPU_SWITCH_ATTEMPTED");
    
    if ((offload == nullptr || std::string(offload) != "1") && attempted == nullptr)
    {
        setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);
        setenv("__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1);
        setenv("__GL_VENDOR_LIBRARY_NAME", "nvidia", 1);
        
        setenv("VORTEX_GPU_SWITCH_ATTEMPTED", "1", 1);

        char buffer[1024];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1)
        {
            buffer[len] = '\0';
            char* argv[] = { buffer, nullptr };
            execv(buffer, argv);
        }
    }
}
#endif

#endif // VORTEX_GPU_PREINIT_H
