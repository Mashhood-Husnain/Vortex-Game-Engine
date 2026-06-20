#include "vortex_audio.hpp"

ma_engine* VortexAudio::sound_engine = nullptr;
bool VortexAudio::initialized = false;
std::vector<ma_sound*> VortexAudio::active_sounds;

void VortexAudio::init()
{
    sound_engine = new ma_engine();
    ma_result result = ma_engine_init(NULL, sound_engine);

    if (result != MA_SUCCESS)
    {
        VORTEX_ERROR("[AUDIO ERROR] Failed to initialize audio engine!");
        exit(EXIT_FAILURE);
    }

    initialized = true;

    VORTEX_INFO("[AUDIO] System initialized successfully.");
}

void VortexAudio::play_sound(const std::string& filepath, float volume)
{
    if (!initialized) return;

    ma_sound* sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(sound_engine, filepath.c_str(), 0, NULL, NULL, sound);

    if (result == MA_SUCCESS)
    {
        ma_sound_set_volume(sound, volume);
        ma_sound_start(sound);

        active_sounds.push_back(sound);
    }
    else
    {
        VORTEX_ERROR("[AUDIO WARNING] Could not load: ", filepath);
        delete sound;
    }
}

void VortexAudio::update()
{
    if (!initialized) return;

    for (auto it = active_sounds.begin(); it != active_sounds.end(); )
    {
        ma_sound* sound = *it;

        if (ma_sound_at_end(sound))
        {
            ma_sound_uninit(sound);
            delete sound;

            it = active_sounds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void VortexAudio::clean_up()
{
    VORTEX_INFO("[AUDIO] Closing hardware audio stream context and unloading sound buffers...");

    if (!initialized) return;

    for (ma_sound* sound : active_sounds)
    {
        ma_sound_uninit(sound);
        delete sound;
    }
    active_sounds.clear();

    ma_engine_uninit(sound_engine);
    delete sound_engine;
    initialized = false;
}
