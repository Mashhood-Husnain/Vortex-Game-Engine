/*
 * File: vortex_ps_test.hpp
 * Project: VortexEngine
 * Description: Tests for Particle System
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include "vortex_particlesystem.hpp"

#include <cstdlib>

void ps_smoke(glm::vec3 position, ParticleSystem &ps, int no_of_particles)
{
    float particle_size = 0.5f;
    float gravity = -0.06f;
    float drag = 0.9f;

    for (int i = 0; i < no_of_particles; i++)
    {
        float angle = (rand() % 360) * (M_PI / 180.0f);
        float spread = (rand() % 100 / 100.0f) * 0.8f;

        float upward_speed = 0.5f; //(rand() % 100 / 40.0f) + 0.8f;
        float particle_life = 2.0f + ((rand() % 100) / 100.0f);

        glm::vec3 spawn_pos = position;
        spawn_pos.x += ((rand() % 100 / 100.0f) - 0.5f) * 0.5f;
        spawn_pos.z += ((rand() % 100 / 100.0f) - 0.5f) * 0.5f;

        glm::vec3 velocity(
            cos(angle) * spread,
            upward_speed,
            sin(angle) * spread
        );

        ps.emit(
            spawn_pos,
            particle_size,
            velocity,
            particle_life,
            gravity,
            drag,
            GLOBAL::COLORS::SMOKE,
            ParticleBehaviour::GROW
        );
    }
}
