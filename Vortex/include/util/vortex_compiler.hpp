#pragma once

#include <mutex>
#include <atomic>
#include <string>
#include <filesystem>
#include <fstream>

#include "vortex_save_load.hpp"
#include "vortex_gui.hpp"
#include "vortex_util.hpp"
#include "vortex_logs.hpp"
#include "vortex_game_api.hpp"
#include "vortex_game_reloader.hpp"
#include "vortex_script_registry.hpp"

class VortexApplication;

struct CompilerState
{
    std::atomic<bool> is_compiling{false};
    std::atomic<float> progress{0.0f};
    std::mutex status_mutex;
    std::string status_text = "Initializing compiler...";
};

class VortexCompiler
{
    static CompilerState compiler_state;
    static GameMemory game_memory;
    static GameCode *game_code;

    static bool initialized;
public:
    static void trigger_compile();
    static void check_for_hot_reload(VortexApplication *app);
    static void update(VortexApplication *app);
    static void init_game_code(VortexApplication *app);

    static bool is_compiling();
    static float progress();
    static std::string status_text();

    static void set_status_mutex(const std::string &status_text);

    static void clean_up();
};