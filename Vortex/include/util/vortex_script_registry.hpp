#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

#include "vortex_behaviour.hpp"

class ScriptRegistry
{
public:
    static ScriptRegistry &get()
    {
        static ScriptRegistry instance;
        return instance;
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

    std::vector<std::string> get_avaialble_scripts()
    {
        std::vector<std::string> names;
        for (const auto &pair : factories)
        {
            names.push_back(pair.first);
        }

        return names;
    }
};


// some macro that links the scripts, fuck me if i know how it works
#define VORTEX_REGISTER_SCRIPT(className) \
    class className##_Registrar { \
    public: \
        className##_Registrar() { \
            ScriptRegistry::get().register_script(#className, []() { return new className(); });\
        } \
    };\
    static className##_Registrar global_##className##_registrar;
