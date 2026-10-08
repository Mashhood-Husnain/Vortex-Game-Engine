#include "VortexEngine.hpp"

extern "C"
{
    VORTEX_API void GameInit(GameMemory *memory, VortexApplication *app)
    {
        ScriptRegistry::set_instance(static_cast<ScriptRegistry*>(memory->registry_context));

        if (!memory->is_initialized)
        {
            VORTEX_INFO_TAG("VORTEX GAME", "Initializing Engine State...");
            memory->is_initialized = true;
        }

        auto& pending = get_pending_scripts();
        for (const auto& script : pending)
        {
            ScriptRegistry::get().register_script(script.name, script.factory);
            VORTEX_INFO_TAG("GAME", "Registered Script: ", script.name);
        }

        pending.clear();
    }

    VORTEX_API void GameUpdate(GameMemory *memory, VortexApplication *app)
    {

    }
}
