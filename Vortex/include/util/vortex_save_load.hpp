#pragma once

#include <string>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

#include <json.hpp>

#include "../vortex_model.hpp"
#include "../vortex_particlesystem.hpp"
#include "../vortex_application.hpp"

#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"

using json = nlohmann::json;

enum class SnapshotState
{
    SAVE,
    LOAD
};

struct SettingsSnapshot
{
    char *preferred_ide_path;
};

struct Snapshot
{
    std::string &project_name;
    std::vector<VortexModel*> &active_models;
    std::vector<ParticleSystem*> &active_systems;
    int &m_selected_skybox_idx;
    int &m_selected_shader_idx;
    std::vector<std::string> explicit_empty_folders;
    VortexApplication *window;
    SettingsSnapshot *settings_snapshot;
};

class VortexProject
{
public:
    inline static const std::string SAVE_DIRECTORY = "saves";
    inline static const std::string ASSET_DIR_SCRIPTS = "Scripts";
    inline static const std::string ASSET_DIR_AUDIO = "Audio";
    inline static const std::string ASSET_DIR_MODELS = "Models";
    inline static const std::string ASSET_DIR_MODELS_OBJ = "obj";
    inline static const std::string ASSET_DIR_MODELS_MTL = "mtl";
    inline static const std::string ASSET_DIR_MODELS_MTL_TEXTURES = "textures";

    static std::vector<std::string> get_project_names();

    static void save_project(Snapshot *snapshot);
    static void load_project(Snapshot *snapshot);

    static void save_project_settings(SettingsSnapshot *settings_snapshot, const std::string project_name);
    static void load_project_settings(SettingsSnapshot *settings_snapshot, const std::string project_name);

    static bool check_save_state(Snapshot *snapshot);
    static void take_snapshot(SnapshotState state, VortexApplication *window, std::string project_name="");
    static void clean_playmode_backups();
};

class VortexEncrypt
{
    static std::string get_key();
public:
    static void write_encrypted(const std::string &file_path, const std::string &data);
    static std::string read_decrypted(const std::string &file_path);
};
