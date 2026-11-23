#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <fstream>

#include <json.hpp>

#include "vortex_global_vars.hpp"
#include "vortex_keyboard.hpp"
#include "vortex_logs.hpp"

namespace
{
    bool is_subset(const std::vector<Key> &small, const std::vector<Key> &big)
    {
        return std::all_of(
            small.begin(),
            small.end(),
            [&](const Key &k) {
                return std::find(big.begin(), big.end(), k) != big.end();
            }
        );
    }
}

struct Shortcut
{
    std::vector<Key> key_comb;
    bool timed = false;

    std::function<bool()> enabled;
    std::function<void()> action;

    std::string name;
    float timed_interval = 0.0f;
};

class VortexShortcuts
{
    inline static std::unordered_map<GLOBAL::EngineState, std::vector<Shortcut>> s_shortcuts;
public:
    static void bind(GLOBAL::EngineState state, Shortcut s);
    static void update(GLOBAL::EngineState state);

    static void save_to_json(const std::string &path);
};
