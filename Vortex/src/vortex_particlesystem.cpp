#include "vortex_particlesystem.hpp"

ParticleSystem::ParticleSystem(int max_particles)
{
    particles.resize(max_particles);
    setup_buffers();
}

void ParticleSystem::emit(glm::vec3 position, float size, glm::vec3 velocity, float life, float gravity_scale, float drag, glm::vec4 particle_color)
{
    for (Particle &particle : particles)
    {
        if (!particle.is_active())
        {
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
            break;
        }
    }
}


void ParticleSystem::update(float deltaTime)
{
    for (Particle &particle : particles)
    {
        if (particle.is_active())
        {
            particle.life -= deltaTime;
            float life_ratio = particle.life / particle.max_life;

            particle.particle_instance.size = particle.initial_size * (life_ratio * life_ratio);

            particle.velocity.y -= (global_WORLDGRAVITY * particle.gravity_scale) * deltaTime;
            particle.velocity *= (1.0f - (particle.drag * deltaTime));
            particle.particle_instance.position += particle.velocity * deltaTime;

            if (particle.gravity_scale < 0.0f)
            {
                float time = glfwGetTime();
                particle.particle_instance.position.x += sin(particle.particle_instance.position.y + time) * 0.01f;
                particle.particle_instance.position.z += cos(particle.particle_instance.position.y + time) * 0.01f;
                particle.particle_instance.color.a = particle.particle_instance.color.a * life_ratio;
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
        }
    }
}

void ParticleSystem::draw(VortexShader &shader, VortexCamera &camera)
{
    std::vector<ParticleInstance> instance_data;
    for (const Particle &particle : particles)
    {
        if (particle.is_active())
        {
            instance_data.push_back(particle.particle_instance);
        }
    }

    if (instance_data.empty()) return;

    glUseProgram(shader.shader_program);

    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f)));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instance_data.size() * sizeof(ParticleInstance), instance_data.data());

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)instance_data.size());
    glBindVertexArray(0);
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
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(ParticleInstance), NULL, GL_DYNAMIC_DRAW);

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
