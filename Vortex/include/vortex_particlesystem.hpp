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
#include "util/vortex_global_vars.hpp"
#include "vortex_window.hpp"

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

struct Particle
{
    glm::vec3 velocity;
    ParticleInstance particle_instance;
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

    bool is_active() const {
        return life > 0.0f && particle_instance.size > 0.01;
    }
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
};

class ParticleSystem
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int instanceVBO;

    void setup_buffers();
public:
    std::vector<Particle> particles;
    std::map<std::string, EmitterSettings> emitter_registry;

    VortexWindow *window = nullptr;
    std::string name;

    int max_particles;
    int active_count = 0;

    ParticleSystem(int max_particles, VortexWindow *window, std::string name);
    void emit(
        glm::vec3 position, float size, glm::vec3 velocity, float life, float gravity_scale, float drag,
        glm::vec4 particle_color, float elasticity, float friction, ParticleBehaviour behaviour=ParticleBehaviour::NONE,
        bool use_point_gravity=false, glm::vec3 gravity_point=glm::vec3(0.0f), float point_gravity_strength=0.0f
    );

    void update(float deltaTime);
    void draw(VortexShader &shader, VortexCamera &camera);
    void resize_particles(int no_of_particles);
    EmitterSettings& get_emitter(std::string name);

    ~ParticleSystem();
};
