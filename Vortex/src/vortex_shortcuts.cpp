#include "vortex_shortcuts.hpp"

void VortexShortcuts::bind(GLOBAL::EngineState state, Shortcut s)
{
    auto &list = s_shortcuts[state];
    list.push_back(std::move(s));

    std::stable_sort(
        list.begin(),
        list.end(),
        [](const Shortcut &a, const Shortcut &b) {
            return a.key_comb.size() > b.key_comb.size();
        }
    );
}

void VortexShortcuts::update(GLOBAL::EngineState state)
{
    std::vector<const std::vector<Key> *> fired;

    for (auto &shortcut : s_shortcuts[state])
    {
        if (shortcut.key_comb.empty()) continue;
        if (shortcut.enabled && !shortcut.enabled()) continue;

        bool suppressed = std::any_of(
            fired.begin(),
            fired.end(),
            [&](const std::vector<Key> *f) {
                return f->size() > shortcut.key_comb.size() && is_subset(shortcut.key_comb, *f);
            }
        );

        if (suppressed) continue;

        bool comb_detected = true;
        for (auto &key : shortcut.key_comb)
        {
            const bool down = shortcut.timed
                ? VortexKeyboard::get_key(key, shortcut.timed_interval, true)
                : VortexKeyboard::get_key(key);

            if (!down)
            {
                comb_detected = false;
                break;
            }
        }

        if (comb_detected)
        {
            shortcut.action();
            return;
        }
    }
}

void VortexShortcuts::save_to_json(const std::string &path)
{
    using json = nlohmann::json;
    json root = json::object();

    for (const auto &[state, shortcuts] : s_shortcuts)
    {
        json shortcut_json = json::array();

        for (const auto &s : shortcuts)
        {
            json keys = json::array();
            for (const auto &key : s.key_comb)
            {
                keys.push_back(VortexKeyboard::str_key(key));
            }

            shortcut_json.push_back({
                {"Name", s.name},
                {"Keys", std::move(keys)},
                {"Timed", s.timed},
                {"TimedInterval", s.timed_interval}
            });
        }

        std::string state_name;
        switch (state)
        {
            case GLOBAL::EngineState::EDITOR: state_name = "Editor"; break;
            case GLOBAL::EngineState::PLAY: state_name = "Play"; break;
            case GLOBAL::EngineState::PROJECT_HUB: state_name = "ProjectHub"; break;
        }
        if (state_name.empty())
        {
            VORTEX_ERROR("Unknow engine state while saving shortcuts (", state_name, ")");
            continue;
        }

        root[state_name] = std::move(shortcut_json);
    }

    std::ofstream file(path);
    if (!file)
    {
        VORTEX_ERROR("Could not open file: ", path);
        return;
    }

    file << root.dump(4);
}
