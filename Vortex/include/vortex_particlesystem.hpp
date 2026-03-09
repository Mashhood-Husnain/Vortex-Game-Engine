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

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "util/vortex_global_vars.hpp"

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

    bool is_active() const {
        return life > 0.0f && particle_instance.size > 0.01;
    }
};

class ParticleSystem
{
    std::vector<Particle> particles;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int instanceVBO;
    int max_particles;
    int active_count = 0;

    void setup_buffers();
public:
    ParticleSystem(int max_particles);
    void emit(
        glm::vec3 position, float size, glm::vec3 velocity, float life, float gravity_scale, float drag,
        glm::vec4 particle_color, ParticleBehaviour behaviour=ParticleBehaviour::NONE
    );
    void update(float deltaTime);
    void draw(VortexShader &shader, VortexCamera &camera);
    void resize_particles(int no_of_particles);
    ~ParticleSystem();
};
