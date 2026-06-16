#pragma once

#ifdef _WIN32
    #define VORTEX_API __declspec(dllexport)
#else
    #define VORTEX_API __attribute__((visibility("default")))
#endif

class VortexApplication;

struct GameMemory
{
    bool is_initialized;
    void *registry_context;
};

extern "C"
{
    typedef void (*GameInitFunc)(GameMemory *memory, VortexApplication *app);
    typedef void (*GameUpdateFunc)(GameMemory *memory, VortexApplication *app);

    VORTEX_API void GameInit(GameMemory *memory, VortexApplication *app);
    VORTEX_API void GameUpdate(GameMemory *memory, VortexApplication *app);
}
