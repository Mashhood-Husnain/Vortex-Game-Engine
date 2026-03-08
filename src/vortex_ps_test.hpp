#pragma once

#include "vortex_particlesystem.hpp"

#include <cstdlib>

void ps_explosion(glm::vec3 position, ParticleSystem& ps, int no_of_particles)
{
    float debris_size = 0.5f;
    float debris_lifetime = 2.0f;
    float debris_gravity = 1.0f;
    float debris_drag = 0.1f;
    float smoke_size = 0.5f;
    float smoke_lifetime = 2.5f;
    float smoke_gravity = -0.1f;
    float smoke_drag = 0.8f;

    for (int i = 0; i < no_of_particles; i++)
    {
        glm::vec3 debris_vel(
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100 - 50) / 10.0f
        );
        ps.emit(
            position,
            debris_size,
            debris_vel,
            debris_lifetime,
            debris_gravity,
            debris_drag,
            COLOR_BROWN
        );

        float offsetX = (rand() % 100 - 50) / 100.0f;
        float offsetY = (rand() % 100 - 50) / 100.0f;
        float offsetZ = (rand() % 100 - 50) / 100.0f;
        glm::vec3 position_rand = position + glm::vec3(offsetX, offsetY, offsetZ);

        glm::vec3 smoke_vel(
            (rand() % 100 -50) / 40.0f,
            (rand() % 100 -50) / 20.0f,
            (rand() % 100 -50) / 40.0f
        );
        ps.emit(
            position_rand,
            smoke_size,
            smoke_vel,
            smoke_lifetime,
            smoke_gravity,
            smoke_drag,
            COLOR_SMOKE
        );
    }
}

void ps_circle(glm::vec3 position, ParticleSystem &ps, int no_of_particles)
{
    for (int i = 0; i < no_of_particles; i++)
    {
        float angle = (rand() % 360) * (M_PI / 180.0f);
        float speed = (rand() % 100 / 20.0f) + 1.0f;

        glm::vec3 velocity(
            cos(angle) * speed,
            0.0f,
            sin(angle) * speed
        );

        ps.emit(position, 2.0f, velocity, 2.0f, 0.0f, 0.0f, COLOR_ORANGE);
    }
}
