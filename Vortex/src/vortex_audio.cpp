#include "vortex_audio.hpp"

ma_engine* VortexAudio::sound_engine = nullptr;
bool VortexAudio::initialized = false;
std::vector<ma_sound*> VortexAudio::active_sounds;
std::map<size_t, ma_sound*> VortexAudio::persistent_sounds;
size_t VortexAudio::next_sound_id = 0;

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

    std::string new_filepath = vortex_generatepath(
        VortexProject::SAVE_DIRECTORY,
        VortexGUI::m_new_project_name,
        VortexProject::ASSET_DIR_AUDIO,
        filepath
    );

    ma_result result = ma_sound_init_from_file(sound_engine, new_filepath.c_str(), 0, NULL, NULL, sound);

    if (result == MA_SUCCESS)
    {
        ma_sound_set_volume(sound, volume);
        ma_sound_start(sound);

        active_sounds.push_back(sound);
    }
    else
    {
        VORTEX_ERROR("[AUDIO WARNING] Could not load: ", new_filepath);
        delete sound;
    }
}

size_t VortexAudio::create_looping_sound(const std::string& filepath, float initial_volume)
{
    if (!initialized) return 0;

    ma_sound* sound = new ma_sound();

    std::string new_filepath = vortex_generatepath(
        VortexProject::SAVE_DIRECTORY,
        VortexGUI::m_new_project_name,
        VortexProject::ASSET_DIR_AUDIO,
        filepath
    );

    ma_result result = ma_sound_init_from_file(sound_engine, new_filepath.c_str(), 0, NULL, NULL, sound);

    if (result == MA_SUCCESS)
    {
        ma_sound_set_volume(sound, initial_volume);
        ma_sound_set_looping(sound, MA_TRUE);

        size_t id = next_sound_id++;
        persistent_sounds[id] = sound;
        return id;
    }
    else
    {
        VORTEX_ERROR("[AUDIO WARNING] Could not load looping sound: ", new_filepath);
        delete sound;
        return 0;
    }
}

void VortexAudio::set_volume(size_t sound_id, float volume)
{
    if (!initialized || sound_id == 0) return;

    auto it = persistent_sounds.find(sound_id);
    if (it != persistent_sounds.end())
    {
        ma_sound_set_volume(it->second, volume);
    }
}

void VortexAudio::stop_sound(size_t sound_id)
{
    if (!initialized || sound_id == 0) return;

    auto it = persistent_sounds.find(sound_id);
    if (it != persistent_sounds.end())
    {
        ma_sound* sound = it->second;
        ma_sound_stop(sound);
        ma_sound_uninit(sound);
        delete sound;
        persistent_sounds.erase(it);
    }
}

void VortexAudio::pause_sound(size_t sound_id)
{
    if (!initialized || sound_id == 0) return;

    auto it = persistent_sounds.find(sound_id);
    if (it != persistent_sounds.end())
    {
        ma_sound_stop(it->second);
    }
}

void VortexAudio::resume_sound(size_t sound_id)
{
    if (!initialized || sound_id == 0) return;

    auto it = persistent_sounds.find(sound_id);
    if (it != persistent_sounds.end())
    {
        ma_sound_start(it->second);
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

    for (auto const& [id, sound] : persistent_sounds)
    {
        ma_sound_uninit(sound);
        delete sound;
    }
    persistent_sounds.clear();

    ma_engine_uninit(sound_engine);
    delete sound_engine;
    initialized = false;
}
