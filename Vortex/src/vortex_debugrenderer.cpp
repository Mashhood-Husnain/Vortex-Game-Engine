#include "vortex_debugrenderer.hpp"

void VortexDebugRenderer::init()
{
    m_shader = new VortexShader("shaders/debug.vert", "shaders/debug.frag");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * 100000, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, color));

    glBindVertexArray(0);
}

void VortexDebugRenderer::draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color)
{
    m_line_vertices.push_back({start, color});
    m_line_vertices.push_back({end, color});
}


void VortexDebugRenderer::draw_sphere(const glm::vec3& center, float radius, const glm::vec3& color)
{
    const int segments = 16;
    const float angle_step = (2.0f * 3.14159f) / segments;

    for (int i = 0; i < segments; i++)
    {
        float angle1 = i * angle_step;
        float angle2 = (i + 1) * angle_step;

        draw_line(
            center + glm::vec3(cos(angle1) * radius, sin(angle1) * radius, 0),
            center + glm::vec3(cos(angle2) * radius, sin(angle2) * radius, 0), color
        );

        draw_line(
            center + glm::vec3(cos(angle1) * radius, 0, sin(angle1) * radius),
            center + glm::vec3(cos(angle2) * radius, 0, sin(angle2) * radius), color
        );

        draw_line(
            center + glm::vec3(0, cos(angle1) * radius, sin(angle1) * radius),
            center + glm::vec3(0, cos(angle2) * radius, sin(angle2) * radius), color
        );
    }
}

void VortexDebugRenderer::render(VortexCamera* camera)
{
    if (m_line_vertices.empty()) return;

    m_shader->use();
    glm::mat4 VP = camera->getProjectionMatrix() * camera->getViewMatrix();
    m_shader->setMat4("VP", VP);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferSubData(GL_ARRAY_BUFFER, 0, m_line_vertices.size() * sizeof(DebugVertex), m_line_vertices.data());

    glDrawArrays(GL_LINES, 0, m_line_vertices.size());

    glBindVertexArray(0);
}

void VortexDebugRenderer::update()
{
    m_line_vertices.clear();
}

void VortexDebugRenderer::clean_up()
{
    if (m_shader)
    {
        delete m_shader;
        m_shader = nullptr;
    }

    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }

    if (VBO != 0)
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
}
