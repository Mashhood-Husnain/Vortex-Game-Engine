#include "vortex_compiler.hpp"

CompilerState VortexCompiler::compiler_state;
GameMemory VortexCompiler::game_memory;
GameCode *VortexCompiler::game_code = nullptr;

bool VortexCompiler::initialized = false;

std::string VortexCompiler::status_text()
{
    std::lock_guard<std::mutex> lock(compiler_state.status_mutex);
    return compiler_state.status_text;
}

void VortexCompiler::set_status_mutex(const std::string &status_text)
{
    std::lock_guard<std::mutex> lock(compiler_state.status_mutex);
    compiler_state.status_text = status_text;
}

bool VortexCompiler::is_compiling()
{
    return compiler_state.is_compiling.load();
}

float VortexCompiler::progress()
{
    return compiler_state.progress.load();
}

void VortexCompiler::clean_up()
{
    if (game_code)
    {
        game_code->unload();
        delete game_code;
        game_code = nullptr;
    }
}

void VortexCompiler::init_game_code(VortexApplication *app)
{
    if (initialized) return;

    game_memory = {};
    game_memory.is_initialized = false;
    game_memory.registry_context = &ScriptRegistry::get();

    #ifdef _WIN32
        std::string game_code_name = "VortexGame.dll";
    #else
        std::string game_code_name = "./libVortexGame.so";
    #endif

    game_code = new GameCode(game_code_name);
    game_code->load();

    if (game_code->is_valid && !game_memory.is_initialized)
    {
        game_code->Init(&game_memory, app);
    }

    initialized = true;
}

void VortexCompiler::update(VortexApplication *app)
{
    if (game_code->is_valid)
    {
        game_code->Update(&game_memory, app);
    }
}

void VortexCompiler::check_for_hot_reload(VortexApplication *app)
{
    if (!initialized) return;
    if (!game_code || !game_code->need_reload()) return;

    VORTEX_INFO("[ENGINE] Recompilation detected! Preparing hot reload...");
    std::error_code ec;

    #ifdef _WIN32
        std::string game_code_name = "VortexGame.dll";
        std::string test_temp_path = "VortexGame_test.dll";
    #else
        std::string game_code_name = "./libVortexGame.so";
        std::string test_temp_path = "./libVortexGame_test.so";
    #endif

    std::filesystem::copy_file(game_code_name, test_temp_path, std::filesystem::copy_options::overwrite_existing, ec);

    int retries = 0;
    while (retries < 10)
    {
        std::filesystem::copy_file(game_code_name, test_temp_path, std::filesystem::copy_options::overwrite_existing, ec);
        if (!ec) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        retries++;
    }

    if (ec)
    {
        VORTEX_WARN("[ENGINE] Compiler is still writing. Waiting...");
        return;
    }
    std::filesystem::remove(test_temp_path, ec);

    for (VortexModel* model : VortexObjectManager::active_models)
    {
        for (VortexMonoBehaviour* script : model->behaviours)
        {
            delete script;
        }
        model->behaviours.clear();
    }

    ScriptRegistry::get().clear();

    game_code->unload();
    game_code->load();

    if (game_code->is_valid)
    {
        game_code->Init(&game_memory, app);

        for (VortexModel* model : VortexObjectManager::active_models)
        {
            std::vector<VortexMonoBehaviour*> restored_behaviours;
            std::vector<std::string> restored_names;

            for (const std::string& script_name : model->script_names)
            {
                VortexMonoBehaviour* fresh_script = ScriptRegistry::get().create(script_name);

                if (fresh_script)
                {
                    fresh_script->vortexGameObject = model;
                    fresh_script->vortexEngine = app;
                    fresh_script->vortexTransform = &model->transform;

                    restored_behaviours.push_back(fresh_script);
                    restored_names.push_back(script_name);
                }
                else
                {
                    VORTEX_WARN("[ENGINE] Script '", script_name, "' is missing from the DLL. Removing from model.");
                }
            }

            model->behaviours = restored_behaviours;
            model->script_names = restored_names;
        }
        VORTEX_INFO("[ENGINE] Hot Reload Complete! All pointers safely restored.");
    }
}

void VortexCompiler::trigger_compile()
{
    if (compiler_state.is_compiling.load()) return;

    compiler_state.progress.store(0.0f);
    compiler_state.is_compiling.store(true);

    set_status_mutex("Starting CMake Build...");

    std::string project_script_dir = vortex_generatepath(
        VortexProject::SAVE_DIRECTORY,
        VortexGUI::m_new_project_name,
        VortexProject::ASSET_DIR_SCRIPTS
    );

    std::string absolute_script_dir = std::filesystem::absolute(project_script_dir).string();

    std::thread([absolute_script_dir]()
    {
        #ifdef _WIN32
            #define POPEN _popen
            #define PCLOSE _pclose
        #else
            #define POPEN popen
            #define PCLOSE pclose
        #endif

        std::string safe_path = absolute_script_dir;
        std::replace(safe_path.begin(), safe_path.end(), '\\', '/');

        std::string compile_cmd = "cmake -DPROJECT_DIR=\"" + safe_path + "\" .. && make VortexGame 2>&1";

        FILE* pipe = POPEN(compile_cmd.c_str(), "r");

        if (pipe)
        {
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string line = buffer;
                if (!line.empty() && line.back() == '\n') line.pop_back();

                set_status_mutex(line);

                size_t bracket_open = line.find('[');
                size_t percent_sign = line.find('%');

                if (bracket_open != std::string::npos && percent_sign != std::string::npos && percent_sign > bracket_open)
                {
                    std::string num_str = line.substr(bracket_open + 1, percent_sign - bracket_open - 1);
                    try {
                        float percent = std::stof(num_str) / 100.0f;
                        compiler_state.progress.store(percent);
                    } catch (...) {}
                }
            }
            PCLOSE(pipe);
        }

        compiler_state.progress.store(1.0f);

        set_status_mutex("Build Complete! Triggering Hot Reload...");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        compiler_state.is_compiling.store(false);

    }).detach();
}
