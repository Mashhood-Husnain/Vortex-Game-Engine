/*
 * File: vortex_ps_test.hpp
 * Project: VortexEngine
 * Description: Tests for Particle System
 * Author: Mashhood Husnain
 * License: MIT
 */

#pragma once

#include "vortex_particlesystem.hpp"
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_window.hpp"

#include <cstdlib>

void ps_smoke(
    glm::vec3 position,
    ParticleSystem &ps,
    VortexShader &shader,
    VortexCamera &camera
)
{
    EmitterSettings settings = ps.get_emitter("Smoke");

    if (settings.enabled)
    {
        for (int i = 0; i < settings.spawn_rate; i++)
        {
            float angle = (rand() % 360) * (M_PI / 180.0f);
            float spread = (rand() % 100 / 100.0f) * 0.8f;

            float upward_speed = 0.5f; //(rand() % 100 / 40.0f) + 0.8f;
            float particle_life = settings.life + ((rand() % 100) / 100.0f);

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
                settings.size,
                velocity,
                particle_life,
                settings.gravity,
                settings.drag,
                settings.color,
                settings.elasticity,
                settings.friction,
                ParticleBehaviour::GROW
            );
        }
    }
    
    ps.update(ps.window->deltaTime);
    ps.draw(shader, camera);
}
