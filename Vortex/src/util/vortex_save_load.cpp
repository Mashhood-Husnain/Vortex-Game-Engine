#include "util/vortex_save_load.hpp"

void VortexProject::save_project(
    const std::string &project_name,
    const std::vector<VortexModel*> &active_models,
    const std::vector<ParticleSystem*> &active_systems
)
{
    if (!std::filesystem::exists("saves"))
    {
        std::filesystem::create_directory("saves");
    }

    json save_data;
    save_data["project_name"] = project_name;

    save_data["models"] = json::array();
    for (VortexModel *model : active_models)
    {
        json j_model;
        j_model["path"] = model->file_path;
        j_model["position"] = {model->position.x, model->position.y, model->position.z};
        j_model["rotation"] = {model->rotation.x, model->rotation.y, model->rotation.z};
        j_model["scale"] = {model->scale.x, model->scale.y, model->scale.z};

        save_data["models"].push_back(j_model);
    }

    save_data["particle_systems"] = json::array();
    for (ParticleSystem *ps : active_systems)
    {
        json j_ps;
        j_ps["name"] = ps->name;
        j_ps["max_particles"] = ps->max_particles;

        if (!ps->emitter_registry.empty())
        {
            auto &emitter = ps->emitter_registry["Default_Emitter"];
            j_ps["emitter_pos"] = {emitter.position.x, emitter.position.y, emitter.position.z};
        }

        save_data["particle_systems"].push_back(j_ps);
    }

    std::string file_path = "saves/" + project_name + ".vtx";
    std::ofstream file(file_path);

    file << save_data.dump(4);
    file.close();

    std::cout << "[PROJECT] Successfully saved to " << file_path << std::endl;
}

void VortexProject::load_project(
    const std::string &project_name,
    std::vector<VortexModel*> &active_models,
    std::vector<ParticleSystem*> &active_systems,
    VortexApplication *window
)
{
    std::string file_path = "saves/" + project_name + ".vtx";

    if (!std::filesystem::exists(file_path))
    {
        std::cout << "[PROJECT ERROR] Save fiel not found: " << file_path << std::endl;
        return;
    }

    std::ifstream file(file_path);

    json save_data;
    file >> save_data;
    file.close();

    for (VortexModel *model : active_models) model->should_destroy = true;
    for (ParticleSystem *ps : active_systems) ps->should_destroy = true;

    active_models.clear();
    active_systems.clear();

    for (const auto &j_model : save_data["models"])
    {
        VortexModel *new_model = new VortexModel(j_model["path"], window);

        new_model->position = glm::vec3(j_model["position"][0], j_model["position"][1], j_model["position"][2]);
        new_model->rotation = glm::vec3(j_model["rotation"][0], j_model["rotation"][1], j_model["rotation"][2]);
        new_model->scale = glm::vec3(j_model["scale"][0], j_model["scale"][1], j_model["scale"][2]);

        active_models.push_back(new_model);
    }

    for (const auto &j_ps : save_data["particle_systems"])
    {
        ParticleSystem *new_ps = new ParticleSystem(j_ps["max_particles"], window, j_ps["name"]);

        auto &emitter = new_ps->get_emitter("Default_Emitter");
        if (j_ps.contains("emitter_pos"))
        {
            emitter.position = glm::vec3(j_ps["emitter_pos"][0], j_ps["emitter_pos"][1], j_ps["emitter_pos"][2]);
        }

        active_systems.push_back(new_ps);
    }

    std::cout << "[PROJECT] Successfully loaded " << project_name << std::endl;
}
