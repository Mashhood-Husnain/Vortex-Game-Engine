#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"

struct Particle
{
    glm::vec3 position;
    glm::vec3 velocity;

    float life;
    float size;

    bool is_active() const {return life > 0.0f;}
};

class ParticleSystem
{
    std::vector<Particle> particles;
    unsigned int VAO;
    unsigned int VBO;
    unsigned int instanceVBO;
    int max_particles;

    void setup_buffers();
public:
    ParticleSystem(int max_particles);
    void emit(glm::vec3 position, glm::vec3 velocity, float life);
    void update(float deltaTime);
    void draw(OpenGLShader &shader, OpenGLCamera &camera);
};
