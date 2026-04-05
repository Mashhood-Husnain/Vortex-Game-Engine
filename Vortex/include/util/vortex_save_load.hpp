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

using json = nlohmann::json;

class VortexProject
{
public:
    static void save_project(
        const std::string &project_name,
        const std::vector<VortexModel*> &active_models,
        const std::vector<ParticleSystem*> &active_systems
    );

    static void load_project(
        const std::string &project_name,
        std::vector<VortexModel*> &active_models,
        std::vector<ParticleSystem*> &active_systems,
        VortexApplication *window
    );
};
