#include "vortex_collider.hpp"
#include "vortex_model.hpp"

VortexBoxCollider::VortexBoxCollider()
{
    min = glm::vec3(0.0f);
    max = glm::vec3(0.0f);
}

void VortexBoxCollider::setup_visual_mesh()
{
    std::vector<glm::vec3> vertices = {
        glm::vec3(min.x, min.y, min.z), // 0: Bottom-Left-Back
        glm::vec3(max.x, min.y, min.z), // 1: Bottom-Right-Back
        glm::vec3(max.x, max.y, min.z), // 2: Top-Right-Back
        glm::vec3(min.x, max.y, min.z), // 3: Top-Left-Back
        glm::vec3(min.x, min.y, max.z), // 4: Bottom-Left-Front
        glm::vec3(max.x, min.y, max.z), // 5: Bottom-Right-Front
        glm::vec3(max.x, max.y, max.z), // 6: Top-Right-Front
        glm::vec3(min.x, max.y, max.z)  // 7: Top-Left-Front
    };

    std::vector<unsigned int> indices = {
        0, 1, 1, 2, 2, 3, 3, 0, // back face
        4, 5, 5, 6, 6, 7, 7, 4, // front face
        0, 4, 1, 5, 2, 6, 3, 7 // connecting edges
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

void VortexBoxCollider::draw(VortexShader &collider_shader, VortexCamera &camera, VortexModel *model)
{
    if (VAO == 0) return;

    collider_shader.use();

    collider_shader.setMat4("view", camera.getViewMatrix());
    collider_shader.setMat4("projection", camera.getProjectionMatrix());

    glm::mat4 collider_matrix = model->model_matrix;
    collider_matrix = glm::scale(collider_matrix, model->collider_scale);

    collider_shader.setMat4("model", collider_matrix);

    collider_shader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

VortexBoxCollider::~VortexBoxCollider()
{
    if (VAO != 0)
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}
