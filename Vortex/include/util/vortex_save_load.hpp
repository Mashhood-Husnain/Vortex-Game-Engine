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

struct Snapshot
{
    std::string &project_name;
    std::vector<VortexModel*> &active_models;
    std::vector<ParticleSystem*> &active_systems;
    int &m_selected_skybox_idx;
    int &m_selected_shader_idx;
    std::vector<std::string> explicit_empty_folders;
    VortexApplication *window;
};

class VortexProject
{    
public:
    static std::vector<std::string> search_save_files(const std::string &file_name="");
    static void save_project(Snapshot *snapshot);
    static void load_project(Snapshot *snapshot);
    static bool check_save_state(Snapshot *snapshot);
    static void take_snapshot(SnapshotState state, VortexApplication *window, std::string project_name="");
};

class VortexEncrypt
{
    static std::string get_key();
public:
    static void write_encrypted(const std::string &file_path, const std::string &data);
    static std::string read_decrypted(const std::string &file_path);
};
