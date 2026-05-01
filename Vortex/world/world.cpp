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

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

int main() {
    
    try
    {
        #ifdef _WIN32
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

    VortexApplication VortexEngineWindow("Vortex Engine");

    VortexEngineWindow.run([&](){});

    return 0;
}
