/*
 * File: vortex_particlesystem.hpp
 * Project: VortexEngine
 * Description: Implementation of particle system
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <map>
#include <string>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_global_vars.hpp"
#include "vortex_application.hpp"

class VortexApplication;
class VortexCamera;
class VortexShader;

enum class ParticleBehaviour
{
    GROW,
    SHRINK,
    NONE
};

struct ParticleInstance
{
    glm::vec3 position;
    float size;
    glm::vec4 color;
};

struct ParticlePhysics
{
    glm::vec3 velocity;
    float life;
    float max_life;
    float initial_size;
    float initial_alpha;

    float gravity_scale;
    float drag;

    float elasticity = 0.5f;
    float friction = 0.8f;

    ParticleBehaviour behaviour;

    bool use_point_gravity;
    glm::vec3 gravity_point;
    float point_gravity_strength;
};

struct EmitterSettings
{
    bool enabled = true;
    int spawn_rate = 1;
    float spawn_timer = 0.0f;
    float size = 0.5f;
    float life = 1.0f;
    float gravity = 0.0f;
    float drag = 0.9f;
    float elasticity = 0.5f;
    float friction = 0.8f;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);

    bool use_point_gravity = false;
    glm::vec3 gravity_point = glm::vec3(0.0f, 10.0f, 0.0f);
    float point_gravity_strength = 0.5f;

    ParticleBehaviour behaviour = ParticleBehaviour::NONE;
};

class ParticleSystem
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int instanceVBO;

    void setup_buffers();
public:
    std::vector<ParticleInstance> instances;
    std::vector<ParticlePhysics> physics;
    std::map<std::string, EmitterSettings> emitter_registry;

    VortexApplication *window = nullptr;
    std::string name;

    int max_particles;
    int active_count = 0;

    bool should_destroy = false;

    ParticleSystem(int max_particles, VortexApplication *window, std::string name);
    void emit(
        glm::vec3 position, float size, glm::vec3 velocity, float life, float gravity_scale, float drag,
        glm::vec4 particle_color, float elasticity, float friction, ParticleBehaviour behaviour=ParticleBehaviour::NONE,
        bool use_point_gravity=false, glm::vec3 gravity_point=glm::vec3(0.0f), float point_gravity_strength=0.0f
    );

    void update();
    void draw(VortexShader &shader, VortexCamera &camera);
    void resize_particles(int no_of_particles);
    EmitterSettings& get_emitter(std::string name);

    ~ParticleSystem();
};
