#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <system_error>

#include "vortex_game_api.hpp"
#include "vortex_logs.hpp"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

class GameCode
{
    #ifdef _WIN32
        HMODULE library_handle = nullptr;
    #else
        void *library_handle = nullptr;
    #endif

    std::filesystem::file_time_type last_write_time;
    std::string dll_path;
    std::string current_temp_path;
    int reload_counter = 0;

public:
    GameInitFunc Init = nullptr;
    GameUpdateFunc Update = nullptr;
    bool is_valid = false;

    GameCode(std::string &path)
    {
        dll_path = path;
        
        std::error_code ec;
        if (std::filesystem::exists(dll_path, ec)) 
        {
            last_write_time = std::filesystem::last_write_time(dll_path, ec);
        }
    }

    void load()
    {
        if (std::filesystem::exists(dll_path))
        {
            std::error_code ec;

            reload_counter++;

            last_write_time = std::filesystem::last_write_time(dll_path, ec);

            #ifdef _WIN32
                current_temp_path = "VortexGame_temp_" + std::to_string(reload_counter) + ".dll";
            #else
                current_temp_path = "./libVortexGame_temp_" + std::to_string(reload_counter) + ".so";
            #endif

            std::filesystem::copy_file(dll_path, current_temp_path, std::filesystem::copy_options::none, ec);
            
            if (ec)
            {
                VORTEX_WARN("[COMPILER] Copy failed, file might be locked. Retrying next frame...");
                return;
            }
        }
        else
        {
            VORTEX_ERROR("[COMPILER] Could not find file: ", dll_path);
            return;
        }

        #ifdef _WIN32
            library_handle = LoadLibrary(current_temp_path.c_str());
            if (library_handle)
            {
                Init = (GameInitFunc)GetProcAddress(library_handle, "GameInit");
                Update = (GameUpdateFunc)GetProcAddress(library_handle, "GameUpdate");
            }
            else
            {
                VORTEX_ERROR("[COMPILER] LoadLibrary failed! Error Code: ", GetLastError());
            }
        #else
            dlerror();
            
            library_handle = dlopen(current_temp_path.c_str(), RTLD_NOW);
            if (library_handle)
            {
                Init = (GameInitFunc)dlsym(library_handle, "GameInit");
                Update = (GameUpdateFunc)dlsym(library_handle, "GameUpdate");
                
                const char* dlsym_error = dlerror();
                if (dlsym_error)
                {
                    VORTEX_ERROR("[COMPILER] dlsym failed: ", dlsym_error);
                }
            }
            else
            {
                VORTEX_ERROR("[COMPILER] dlopen failed: ", dlerror());
            }
        #endif
        
        is_valid = (Init != nullptr && Update != nullptr);
        if (is_valid)
        {
            VORTEX_INFO("[COMPILER] Game Code Loaded Successfully! (Gen ", reload_counter, ")");
        }
        else
        {
            VORTEX_WARN("[COMPILER] Failed to load Game Code!");
        }
    }

    void unload()
    {
        if (library_handle)
        {
            #ifdef _WIN32
                FreeLibrary(library_handle);
            #else
                dlclose(library_handle);
            #endif
            library_handle = nullptr;
        }

        if (!current_temp_path.empty())
        {
            std::error_code ec;
            std::filesystem::remove(current_temp_path, ec);
        }

        Init = nullptr;
        Update = nullptr;
        is_valid = false;
    }

    bool need_reload()
    {
        std::error_code ec;
        
        if (!std::filesystem::exists(dll_path, ec)) return false;

        auto current_time = std::filesystem::last_write_time(dll_path, ec);
        if (ec) return false;

        if (current_time > last_write_time)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            last_write_time = std::filesystem::last_write_time(dll_path, ec);
            
            return true;
        }

        return false;
    }
};
