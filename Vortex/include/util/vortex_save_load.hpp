#pragma once

#include <string>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

#include <json.hpp>

#include "../vortex_model.hpp"
#include "../vortex_particlesystem.hpp"
#include "../vortex_application.hpp"

#include "vortex_behaviour.hpp"
#include "vortex_script_registry.hpp"

using json = nlohmann::json;

struct SaveScene_snapshot
{
    std::string &project_name;
    std::vector<VortexModel*> &active_models;
    std::vector<ParticleSystem*> &active_systems;
    int &m_selected_skybox_idx;
    int &m_selected_shader_idx;
    VortexApplication *window;
};

class VortexProject
{    
public:
    static std::vector<std::string> search_save_files(const std::string &file_name="");
    static void save_project(SaveScene_snapshot *snapshot);
    static void load_project(SaveScene_snapshot *snapshot);
    static bool check_save_state(SaveScene_snapshot *snapshot);
};
