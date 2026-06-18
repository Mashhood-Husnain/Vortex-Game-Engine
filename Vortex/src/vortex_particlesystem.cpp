/*
 * File: vortex_particlesystem.cpp
 * Project: VortexEngine
 * Description: Implementation of particle system
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_particlesystem.hpp"

ParticleSystem::ParticleSystem(int max_particles, VortexApplication *window, std::string name)
{
    this->window = window;
    this->name = name;
    this->max_particles = max_particles;

    instances.resize(this->max_particles);
    physics.resize(this->max_particles);

    setup_buffers();
}


void ParticleSystem::emit(const EmitterSettings& settings, glm::vec3 initial_velocity)
{
    if (active_count >= max_particles) return;

    ParticleInstance &inst = instances[active_count];
    ParticlePhysics &phys = physics[active_count];

    inst.position = settings.position;
    inst.size = settings.size;
    inst.color = settings.start_color;

    phys.velocity = initial_velocity;
    phys.life = settings.life;
    phys.max_life = settings.life;
    phys.initial_size = settings.size;

    phys.start_color = settings.start_color;
    phys.end_color = settings.end_color;

    phys.gravity_scale = settings.gravity;
    phys.drag = settings.drag;
    phys.elasticity = settings.elasticity;
    phys.friction = settings.friction;
    phys.turbulence_strength = settings.turbulence;
    phys.behaviour = settings.behaviour;

    phys.use_point_gravity = settings.use_point_gravity;
    phys.gravity_point = settings.gravity_point;
    phys.point_gravity_strength = settings.point_gravity_strength;

    active_count++;
}

void ParticleSystem::burst(std::string emitter_name, int count)
{
    EmitterSettings& settings = get_emitter(emitter_name);

    for (int i = 0; i < count; i++)
    {
        float phi = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
        float costheta = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        float u = ((float)rand() / RAND_MAX);

        float theta = acos(costheta);
        float r = cbrt(u);

        glm::vec3 velocity(
            r * sin(theta) * cos(phi),
            r * sin(theta) * sin(phi),
            r * cos(theta)
        );

        velocity *= (1.0f + ((rand() % 100) / 100.0f) * 5.0f);

        emit(settings, velocity);
    }
}

void ParticleSystem::update()
{
    for (auto &[name, settings] : emitter_registry)
    {
        if (settings.enabled && settings.spawn_rate > 0)
        {
            float spawn_interval = 1.0f / static_cast<float>(settings.spawn_rate);
            settings.spawn_timer += GLOBAL::deltaTime;

            while (settings.spawn_timer >= spawn_interval)
            {
                float angle = static_cast<float>(rand() % 360) * (static_cast<float>(M_PI) / 180.0f);
                glm::vec3 velocity(cos(angle), 1.0f, sin(angle));

                emit(settings, velocity);
                settings.spawn_timer -= spawn_interval;
            }
        }
    }

    float g_dt = GLOBAL::GRAVITY * GLOBAL::deltaTime;

    for (int i = 0; i < active_count; )
    {
        ParticleInstance &inst = instances[i];
        ParticlePhysics &phys = physics[i];

        phys.life -= GLOBAL::deltaTime;

        if (phys.life <= 0.0f)
        {
            instances[i] = instances[active_count - 1];
            physics[i] = physics[active_count - 1];
            active_count--;
            continue;
        }

        float life_ratio = phys.life / phys.max_life;

        inst.color = glm::mix(phys.end_color, phys.start_color, life_ratio);

        switch (phys.behaviour)
        {
            case ParticleBehaviour::GROW:
                inst.size = phys.initial_size + (phys.initial_size * (1.0f - life_ratio));
                break;
            case ParticleBehaviour::SHRINK:
                inst.size = phys.initial_size * (life_ratio * life_ratio);
                break;
            default:
                inst.size = phys.initial_size;
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

        if (phys.turbulence_strength > 0.0f)
        {
            float time_offset = GLOBAL::deltaTime * 5.0f;
            phys.velocity.x += sin(inst.position.y * 2.0f + time_offset) * phys.turbulence_strength;
            phys.velocity.z += cos(inst.position.x * 2.0f + time_offset) * phys.turbulence_strength;
        }

        phys.velocity *= 1.0f - (phys.drag * GLOBAL::deltaTime);
        inst.position += phys.velocity * GLOBAL::deltaTime;

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

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, draw_count * sizeof(ParticleInstance), instances.data());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    shader.setInt("particleTexture", 0);
    glActiveTexture(GL_TEXTURE0);

    if (emitter_registry["Default Emitter"].texture_id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, emitter_registry["Default Emitter"].texture_id);
        shader.setBool("useTexture", true);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setBool("useTexture", false);
    }

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // UVs for the particle texture
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Instance Data
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(ParticleInstance), NULL, GL_DYNAMIC_DRAW);

    // Position (Attribute 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, position));
    glVertexAttribDivisor(2, 1);

    // Size (Attribute 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, size));
    glVertexAttribDivisor(3, 1);

    // Color (Attribute 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInstance), (void*)offsetof(ParticleInstance, color));
    glVertexAttribDivisor(4, 1);

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