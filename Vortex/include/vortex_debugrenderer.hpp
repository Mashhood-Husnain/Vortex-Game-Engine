#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"

struct DebugVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class VortexDebugRenderer
{
    std::vector<DebugVertex> m_line_vertices;
    GLuint VAO, VBO;
    VortexShader *m_shader;

    VortexDebugRenderer() {};
public:
    VortexDebugRenderer(const VortexDebugRenderer&) = delete;
    void operator=(const VortexDebugRenderer&) = delete;

    static VortexDebugRenderer &get()
    {
        static VortexDebugRenderer instance;
        return instance;
    }

    void init();
    void draw_line(const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &color);
    void draw_sphere(const glm::vec3 &center, float radius, const glm::vec3 &color);

    void render(VortexCamera *camera);
    void clear();

    ~VortexDebugRenderer();
};
