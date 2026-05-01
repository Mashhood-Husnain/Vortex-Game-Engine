#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

#include "vortex_behaviour.hpp"

class ScriptRegistry
{
    inline static ScriptRegistry* s_Instance = nullptr;
public:
    static ScriptRegistry& get()
    {
        if (s_Instance == nullptr) {
            s_Instance = new ScriptRegistry();
        }
        return *s_Instance;
    }

    static void set_instance(ScriptRegistry* instance)
    {
        s_Instance = instance;
    }

    std::unordered_map<std::string, std::function<VortexMonoBehaviour*()>> factories;

    void register_script(const std::string &name, std::function<VortexMonoBehaviour*()> factory)
    {
        factories[name] = factory;
    }

    VortexMonoBehaviour* create(const std::string &name)
    {
        if (factories.find(name) != factories.end())
        {
            return factories[name]();
        }
        return nullptr;
    }

    std::vector<std::string> get_available_scripts()
    {
        std::vector<std::string> names;
        for (const auto &pair : factories)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    void clear()
    {
        factories.clear();
    }
};

struct PendingScript
{
    std::string name;
    std::function<VortexMonoBehaviour*()> factory;
};

inline std::vector<PendingScript> &get_pending_scripts()
{
    static std::vector<PendingScript> pending;
    return pending;
}

#define VORTEX_REGISTER_SCRIPT(className) \
    class className##_Registrar { \
    public: \
        className##_Registrar() { \
            get_pending_scripts().push_back({#className, []() -> VortexMonoBehaviour* { return new className(); }}); \
        } \
    }; \
    static className##_Registrar global_##className##_registrar;
