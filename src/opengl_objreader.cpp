#include "opengl_objreader.hpp"

OpenGLModel::OpenGLModel(const std::string& path)
{
    load_obj(path);
    setup_mesh();
}
void OpenGLModel::draw(const OpenGLShader& shader, OpenGLCamera& camera)
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
    glUniform4f(glGetUniformLocation(shader.shader_program, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    if (texture_id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, texture_id);
    }

    // // temporary 
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glLineWidth(2.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);

    // // temporary 
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLModel::load_obj(const std::string& path)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_tex_coords;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open OBJ: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v")
        {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            temp_positions.push_back(v);
        } 
        else if (prefix == "vt")
        {
            glm::vec2 vt;
            ss >> vt.x >> vt.y;
            temp_tex_coords.push_back(vt);
        } 
        else if (prefix == "vn")
        {
            glm::vec3 vn;
            ss >> vn.x >> vn.y >> vn.z;
            temp_normals.push_back(vn);
        } 
        else if (prefix == "f")
        {
            for (int i = 0; i < 3; i++)
            {
                std::string vertexData;
                ss >> vertexData;
                
                // OBJ indices are 1-based, so we subtract 1
                char slash;
                std::stringstream vss(vertexData);
                
                std::string vStr, vtStr, vnStr;
                std::getline(vss, vStr, '/');
                std::getline(vss, vtStr, '/');
                std::getline(vss, vnStr, '/');

                int vIdx  = std::stoi(vStr);
                int vtIdx = vtStr.empty() ? 0 : std::stoi(vtStr);
                int vnIdx = vnStr.empty() ? 0 : std::stoi(vnStr);

                Vertex v;
                v.position = temp_positions[vIdx - 1];

                if (vtIdx > 0 && vtIdx <= temp_tex_coords.size())
                {
                    v.tex_coords = temp_tex_coords[vtIdx - 1];
                }
                else
                {
                    v.tex_coords = glm::vec2(0.0f, 0.0f);
                }

                if (vnIdx > 0 && vnIdx <= temp_normals.size())
                {
                    v.normal = temp_normals[vnIdx - 1];
                }
                else
                {
                    v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                vertices.push_back(v);
            }
        }
    }
}

void OpenGLModel::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position (Location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normal (Location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));

    // TexCoords (Location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(glm::vec3)));

    glBindVertexArray(0);
}
