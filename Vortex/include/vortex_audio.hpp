#pragma once
#include <string>
#include <map>

#include "miniaudio.h"
#include <iostream>
#include <vector>

#include "util/vortex_logs.hpp"
#include "vortex_save_load.hpp"
#include "vortex_util.hpp"
#include "vortex_gui.hpp"

class VortexAudio {
private:
    static ma_engine* sound_engine;
    static bool initialized;

    static std::vector<ma_sound*> active_sounds;

    static std::map<size_t, ma_sound*> persistent_sounds;
    static size_t next_sound_id;

public:
    static void init();
    static void clean_up();
    static void update();

    static void play_sound(const std::string& filepath, float volume = 1.0f);
    static size_t create_looping_sound(const std::string& filepath, float initial_volume);

    static void set_volume(size_t sound_id, float volume);
    static void stop_sound(size_t sound_id);

    static void pause_sound(size_t sound_id);
    static void resume_sound(size_t sound_id);
};
