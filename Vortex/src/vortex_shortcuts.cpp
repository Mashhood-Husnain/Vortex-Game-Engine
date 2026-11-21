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
