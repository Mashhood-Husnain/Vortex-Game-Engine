#include "opengl_stlreader.hpp"

STLModel::STLModel(const std::string& model_path)
{
    load_stl(model_path);
    setup_mesh();
}

void STLModel::draw(const openGLShader& shader, OpenGLCamera& camera)
{
    glUseProgram(shader.shader_program);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f);
    glm::vec3 light_pos(10.0f, 10.0f, 10.0f);

    model_matrix = glm::translate(model_matrix, position);
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model_matrix = glm::scale(model_matrix, glm::vec3(scale));

    glUniform3fv(glGetUniformLocation(shader.shader_program, "lightPos"), 1, glm::value_ptr(light_pos));
    glUniform3fv(glGetUniformLocation(shader.shader_program, "viewPos"), 1, glm::value_ptr(camera.position));
    glUniform4f(glGetUniformLocation(shader.shader_program, "u_Color"), 1.0f, 0.5f, 0.2f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // // temporary 
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glLineWidth(2.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    // // temporary 
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void STLModel::load_stl(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::cerr << "Could not open STL file: " << path << std::endl;
        return;
    }

    char header[5];
    file.read(header, 5);
    file.close();

    if (std::string(header, 5) == "solid") load_ascii(path);
    else load_binary(path);
}

void STLModel::load_binary(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    file.seekg(80);

    uint32_t numTriangles;
    file.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));

    for (uint32_t i = 0; i < numTriangles; i++)
    {
        float n[3], v1[3], v2[3], v3[3];
        uint16_t attribute_byte_count;

        file.read(reinterpret_cast<char*>(n), 12);
        file.read(reinterpret_cast<char*>(v1), 12);
        file.read(reinterpret_cast<char*>(v2), 12);
        file.read(reinterpret_cast<char*>(v3), 12);
        file.read(reinterpret_cast<char*>(&attribute_byte_count), 2);

        glm::vec3 normal_vector(n[0], n[1], n[2]);
        glm::vec3 vertice_1(glm::vec3(v1[0], v1[1], v1[2]));
        glm::vec3 vertice_2(glm::vec3(v2[0], v2[1], v2[2]));
        glm::vec3 vertice_3(glm::vec3(v3[0], v3[1], v3[2]));

        vertices.push_back({vertice_1, normal_vector});
        vertices.push_back({vertice_2, normal_vector});
        vertices.push_back({vertice_3, normal_vector});
    }
}

void STLModel::load_ascii(const std::string& path)
{
    std::ifstream file(path);
    std::string word;
    glm::vec3 current_normal(0.0f);

    while(file >> word)
    {
        if (word == "facet")
        {
            file >> word;
            file >> current_normal.x >> current_normal.y, current_normal.z;
        }
        else if (word == "vertex")
        {
            float x, y, z;
            file >>x >> y >> z;
            vertices.push_back({glm::vec3(x, y, z), current_normal});
        }
    }
}

void STLModel::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));

    glBindVertexArray(0);
}
