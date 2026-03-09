/*
 * File: vortex_particlesystem.cpp
 * Project: VortexEngine
 * Description: Implementation of particle system
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_particlesystem.hpp"

ParticleSystem::ParticleSystem(int max_particles)
{
    this->max_particles = max_particles;
    particles.resize(this->max_particles);
    setup_buffers();
}

void ParticleSystem::emit(glm::vec3 position, float size, glm::vec3 velocity, float life, float gravity_scale, float drag, glm::vec4 particle_color, ParticleBehaviour behaviour)
{
    if (active_count >= particles.size()) return;

    Particle &particle = particles[active_count];

    particle.particle_instance.position = position;
    particle.particle_instance.size = size;
    particle.initial_size = size;
    particle.velocity = velocity;
    particle.life = life;
    particle.max_life = life;
    particle.gravity_scale = gravity_scale;
    particle.drag = drag;
    particle.particle_instance.color = particle_color;
    particle.initial_alpha = particle.particle_instance.color.a;
    particle.behaviour = behaviour;

    active_count++;
}

void ParticleSystem::update(float deltaTime)
{
    float g_dt = global_WORLDGRAVITY * deltaTime;

    for (int i = 0; i < active_count; )
    {
        Particle &particle = particles[i];
        particle.life -= deltaTime;

        float life_ratio = particle.life / particle.max_life;
        float particle_size_behaviour;

        if (particle.life <= 0.0f)
        {
            particles[i] = particles[active_count - 1];
            active_count--;

            continue;
        }

        switch (particle.behaviour)
        {
            case ParticleBehaviour::GROW:
                particle_size_behaviour = particle.initial_size + (particle.initial_size * (1.0f - life_ratio));
            break;

            case ParticleBehaviour::SHRINK:
                particle_size_behaviour = particle.initial_size * (life_ratio * life_ratio);
            break;
        
            default:
                particle_size_behaviour = particle.initial_size;
            break;
        }

        particle.velocity.y -= g_dt * particle.gravity_scale;
        particle.velocity *= 1.0f - (particle.drag * deltaTime);
        particle.particle_instance.position += particle.velocity * deltaTime;
        particle.particle_instance.size = particle_size_behaviour;

        if (particle.gravity_scale < 0.0f)
        {
            float jitterX = ((rand() % 100) / 100.0f - 0.5f) * 0.005f;
            float jitterZ = ((rand() % 100) / 100.0f - 0.5f) * 0.005f;
            
            particle.particle_instance.position.x += jitterX;
            particle.particle_instance.position.z += jitterZ;

            particle.particle_instance.color.a = particle.initial_alpha * life_ratio;
        }

        if (particle.gravity_scale > 0.0f && particle.particle_instance.position.y < 0.0f)
        {
            particle.particle_instance.position.y = 0.0f;
            particle.velocity.y = -particle.velocity.y * particle.elasticity;

            particle.velocity.x *= particle.friction;
            particle.velocity.z *= particle.friction;

            if (glm::length(particle.velocity) < 0.1f)
            {
                particle.velocity = glm::vec3(0.0f);
            }
        }

        i++;
    }
}

void ParticleSystem::draw(VortexShader &shader, VortexCamera &camera)
{
    if (active_count == 0) return;

    static std::vector<ParticleInstance> instance_data;
    instance_data.clear();
    instance_data.reserve(particles.size());

    for (int i = 0; i < active_count; i++)
    {
        instance_data.push_back(particles[i].particle_instance);
    }

    if (instance_data.empty()) return;

    glUseProgram(shader.shader_program);

    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instance_data.size() * sizeof(ParticleInstance), instance_data.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)instance_data.size());

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

    particles.clear();
    particles.resize(max_particles);

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(global_DEFAULTPARTICLEVERTICES), global_DEFAULTPARTICLEVERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // instance location
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(ParticleInstance), NULL, GL_DYNAMIC_DRAW);

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

ParticleSystem::~ParticleSystem()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    if (instanceVBO != 0) glDeleteBuffers(1, &instanceVBO);
    active_count = 0;
    particles.clear();
}
