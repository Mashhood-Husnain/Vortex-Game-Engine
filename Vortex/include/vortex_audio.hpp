#pragma once
#include <string>
#include <map>

#include "miniaudio.h"
#include <iostream>
#include <vector>

class VortexAudio {
private:
    static ma_engine* sound_engine;
    static bool initialized;
    static std::vector<ma_sound*> active_sounds;
public:
    static void init();
    static void clean_up();
    static void update();

    static void play_sound(const std::string& filepath, float volume = 1.0f);
};
