/*
 * File: vortex_particlesystem.cpp
 * Project: VortexEngine
 * Description: Implementation of particle system
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_particlesystem.hpp"

ParticleSystem::ParticleSystem(int max_particles, VortexWindow *window, std::string name)
{
    this->window = window;
    this->name = name;
    this->max_particles = max_particles;

    instances.resize(this->max_particles);
    physics.resize(this->max_particles);

    setup_buffers();
}

void ParticleSystem::emit(
    glm::vec3 position,
    float size,
    glm::vec3 velocity,
    float life,
    float gravity_scale,
    float drag,
    glm::vec4 particle_color,
    float elasticity,
    float friction,
    ParticleBehaviour behaviour,
    bool use_point_gravity,
    glm::vec3 gravity_point,
    float point_gravity_strength
)
{
    if (active_count >= max_particles) return;

    ParticleInstance &inst = instances[active_count];
    ParticlePhysics &phys = physics[active_count];

    inst.position = position;
    inst.size = size;
    inst.color = particle_color;

    phys.initial_size = size;
    phys.velocity = velocity;
    phys.life = life;
    phys.max_life = life;
    phys.gravity_scale = gravity_scale;
    phys.drag = drag;
    phys.initial_alpha = particle_color.a;
    phys.behaviour = behaviour;
    phys.elasticity = elasticity;
    phys.friction = friction;
    phys.use_point_gravity = use_point_gravity;
    phys.gravity_point = gravity_point;
    phys.point_gravity_strength = point_gravity_strength;

    active_count++;
}

void ParticleSystem::update(float deltaTime)
{
    float g_dt = GLOBAL::GRAVITY * deltaTime;

    for (int i = 0; i < active_count; )
    {
        ParticleInstance &inst = instances[i];
        ParticlePhysics &phys = physics[i];

        phys.life -= deltaTime;

        float life_ratio = phys.life / phys.max_life;
        float particle_size_behaviour;

        if (phys.life <= 0.0f)
        {
            instances[i] = instances[active_count - 1];
            physics[i] = physics[active_count - 1];

            active_count--;

            continue;
        }

        switch (phys.behaviour)
        {
            case ParticleBehaviour::GROW:
                particle_size_behaviour = phys.initial_size + (phys.initial_size * (1.0f - life_ratio));
            break;

            case ParticleBehaviour::SHRINK:
                particle_size_behaviour = phys.initial_size * (life_ratio * life_ratio);
            break;
        
            default:
                particle_size_behaviour = phys.initial_size;
            break;
        }

        if (phys.use_point_gravity)
        {
            glm::vec3 direction = phys.gravity_point - inst.position;

            float distance = glm::length(direction);

            if (distance > 0.1f)
            {
                direction = glm::normalize(direction);
                phys.velocity += direction * phys.point_gravity_strength;
            }
        }
        else
        {
            phys.velocity.y -= g_dt * phys.gravity_scale;
        }

        phys.velocity *= 1.0f - (phys.drag * deltaTime);
        inst.position += phys.velocity * deltaTime;
        inst.size = particle_size_behaviour;

        if (phys.gravity_scale < 0.0f)
        {
            float jitterX = ((rand() % 100) / 100.0f - 0.5f) * 0.005f;
            float jitterZ = ((rand() % 100) / 100.0f - 0.5f) * 0.005f;
            
            inst.position.x += jitterX;
            inst.position.z += jitterZ;

            inst.color.a = phys.initial_alpha * life_ratio;
        }

        if (phys.gravity_scale > 0.0f && inst.position.y < 0.0f)
        {
            inst.position.y = 0.0f;
            phys.velocity.y = -phys.velocity.y * phys.elasticity;

            phys.velocity.x *= phys.friction;
            phys.velocity.z *= phys.friction;

            if (glm::length(phys.velocity) < 0.1f)
            {
                phys.velocity = glm::vec3(0.0f);
            }
        }

        i++;
    }

    window->gui.show_inspector_info(nullptr, this);
}

void ParticleSystem::draw(VortexShader &shader, VortexCamera &camera)
{
    if (active_count == 0) return;

    int draw_count = active_count;

    for (auto &[name, settings] : emitter_registry)
    {
        if (settings.enabled && settings.use_point_gravity && draw_count < max_particles)
        {
            ParticleInstance gizmo;
            gizmo.position = settings.gravity_point;
            gizmo.size = 1.1f;
            gizmo.color = glm::vec4(0.0f, 1.0f, 0.0f, 0.9f);

            instances[draw_count] = gizmo;
            draw_count++;
        }
    }

    if (draw_count == 0) return;

    shader.use();
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(ParticleInstance), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, draw_count * sizeof(ParticleInstance), instances.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, draw_count);

    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::resize_particles(int no_of_particles)
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (instanceVBO != 0) glDeleteBuffers(1, &instanceVBO);

    max_particles = no_of_particles;
    active_count = 0;

    instances.clear();
    instances.resize(max_particles);

    physics.clear();
    physics.resize(max_particles);

    setup_buffers();
}

void ParticleSystem::setup_buffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    // Quad location
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLOBAL::DEFAULT_VERTICES::PARTICLE_VERTICES), GLOBAL::DEFAULT_VERTICES::PARTICLE_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // instance location
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(ParticleInstance), NULL, GL_STREAM_DRAW);

    // position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, position)); 
    glVertexAttribDivisor(1, 1);

    // size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, size)); 
    glVertexAttribDivisor(2, 1);
    
    // color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, color));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

EmitterSettings& ParticleSystem::get_emitter(std::string name)
{
    if (emitter_registry.find(name) == emitter_registry.end())
    {
        emitter_registry[name] = EmitterSettings();
    }

    return emitter_registry[name];
}

ParticleSystem::~ParticleSystem()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (instanceVBO != 0) glDeleteBuffers(1, &instanceVBO);
    active_count = 0;
    instances.clear();
    physics.clear();
}
