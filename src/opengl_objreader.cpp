#include "opengl_objreader.hpp"

OpenGLModel::OpenGLModel(const std::string& path)
{
    load_obj(path);
    setup_mesh();
}

void OpenGLModel::draw(const OpenGLShader& shader, OpenGLCamera& camera, bool wireframe)
{
    glUseProgram(shader.shader_program);

    // --- 1. Matrices ---
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model_matrix = glm::rotate(model_matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model_matrix = glm::scale(model_matrix, glm::vec3(scale));

    // --- 2. Uniforms ---
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f)));
    
    glm::vec3 light_pos(10.0f, 10.0f, 10.0f);
    glUniform3fv(glGetUniformLocation(shader.shader_program, "lightPos"), 1, glm::value_ptr(light_pos));
    glUniform3fv(glGetUniformLocation(shader.shader_program, "viewPos"), 1, glm::value_ptr(camera.position));
    // glUniform4f(glGetUniformLocation(shader.shader_program, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 3. Textures (Multi-Sampling) ---
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasTexture"), texture_id != 0);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasRoughness"), roughness_id != 0);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasMetallic"), metallic_id != 0);

    // Slot 0: Diffuse
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id); // If 0, it unbinds
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_diffuseMap"), 0);

    // Slot 1: Roughness
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, roughness_id);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_roughnessMap"), 1);

    // Slot 2: Metallic
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallic_id);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_metallicMap"), 2);

    // --- 4. Rendering Mode ---
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // --- 5. Draw ---
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    glBindVertexArray(0);

    // Reset state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLModel::parse_mtl(const std::string& mtl_filepath)
{
    size_t last_slash = mtl_filepath.find_last_of("/\\");
    std::string mtl_dir = (last_slash == std::string::npos) ? "" : mtl_filepath.substr(0, last_slash + 1);

    std::ifstream file(mtl_filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open MTL: " << mtl_filepath << std::endl;
        return;
    }

    std::string line;
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "map_Kd")
        {
            std::string tex_filename;
            ss >> tex_filename;

            std::string full_tex_path = mtl_dir + tex_filename;
            std::cout << "Loading Texture: " << full_tex_path << std::endl;
            texture_id = load_texture(full_tex_path.c_str());
        }
        else if (prefix == "map_Ns")
        {
            std::string rough_filename;
            ss >> rough_filename;

            std::string full_rough_path = mtl_dir + rough_filename;
            std::cout << "Loading Rougness: " << full_rough_path << std::endl;
            roughness_id = load_texture(full_rough_path.c_str());
        }
        else if (prefix == "map_refl")
        {
            std::string metallic_filename;
            ss >> metallic_filename;

            std::string full_metallic_path = mtl_dir + metallic_filename;
            std::cout << "Loading Metallic: " << full_metallic_path << std::endl;
            metallic_id = load_texture(full_metallic_path.c_str());
        }
        else if (prefix == "map_Bump")
        {
            std::string normal_filename;
            while(ss >> normal_filename);

            std::string full_normal_path = mtl_dir + normal_filename;
            std::cout << "Loading Normal: " << full_normal_path << std::endl;
            normal_id = load_texture(full_normal_path.c_str());
        }
    }
}

unsigned int OpenGLModel::load_texture(const std::string& path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); 
    
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parameters for wrapping and filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else
    {
        std::cerr << "Texture failed to load at: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void OpenGLModel::load_obj(const std::string& path)
{
    size_t last_slash = path.find_last_of("/\\");
    std::string obj_dir = (last_slash == std::string::npos) ? "" : path.substr(0, last_slash + 1);
    std::string filename = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);

    size_t last_dot = filename.find_last_of(".");
    if (last_dot == std::string::npos)
    {
        model_name = filename;
    } else
    {
        model_name = filename.substr(0, last_dot);
    }

    std::cout << "Loading model: " << model_name << " from " << obj_dir << std::endl;

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

        if(prefix == "mtllib")
        {
            std::string mtl_filename;
            ss >> mtl_filename;

            std::string path1 = obj_dir + mtl_filename;            
            std::string path2 = obj_dir + "../mtl/" + mtl_filename;

            std::ifstream check_file(path1);
            if (check_file.is_open()) {
                check_file.close();
                parse_mtl(path1);
            } else {
                parse_mtl(path2);
            }
        }
        else if (prefix == "v")
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
