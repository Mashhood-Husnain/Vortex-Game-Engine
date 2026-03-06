#include "particle_system.hpp"

ParticleSystem::ParticleSystem(int max_particles)
{
    particles.resize(max_particles);
    setup_buffers();
}

void ParticleSystem::emit(glm::vec3 position, glm::vec3 velocity, float life)
{
    for (Particle &p : particles)
    {
        if (!p.is_active())
        {
            p.position = position;
            p.velocity = velocity;
            p.life = life;
            break;
        }
    }
}


void ParticleSystem::update(float deltaTime)
{
    for (Particle &p : particles)
    {
        if (p.is_active())
        {
            p.life -= deltaTime;
            p.position += p.velocity * deltaTime;
            p.velocity.y -= 9.8f * deltaTime;
        }
    }
}

void ParticleSystem::draw(OpenGLShader &shader, OpenGLCamera &camera)
{

    std::vector<glm::vec3> active_positions;
    for (const Particle &p : particles)
    {
        if (p.is_active())
        {
            active_positions.push_back(p.position);
        }
    }

    if (active_positions.empty()) return;

    glUseProgram(shader.shader_program);

    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f)));

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, active_positions.size() * sizeof(glm::vec3), active_positions.data());

    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)active_positions.size());
    glBindVertexArray(0);
}

void ParticleSystem::setup_buffers()
{
    float quad_vertices[] = {
        -0.05f, 0.05f, 0.0f,
        -0.05f, -0.05f, 0.0f,
        0.05f, -0.05f, 0.0f,

        -0.05f, 0.05f, 0.0f,
        0.05f, -0.05f, 0.0f,
        0.05f, 0.05f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    
    glVertexAttribDivisor(1, 1);

    glBindVertexArray(0);
}
