#include "vortex_objreader.hpp"

VortexModel::VortexModel(const std::string& path)
{
    load_obj(path);
    setup_mesh();

    if (!vertices.empty())
    {
        std::cout << "[MODEL] Success: '" << model_name << "' loaded." << std::endl;
        std::cout << "[MODEL] Total Objects Loaded: " << objects.size() << std::endl;
        std::cout << "        -> Vertices: " << vertices.size() << std::endl;
        std::cout << "        -> VAO ID:   " << VAO << std::endl;
    }
}

void VortexModel::draw(const VortexShader& shader, VortexCamera& camera, bool wireframe)
{
    glUseProgram(shader.shader_program);

    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f)));

    glm::vec3 light_pos(10.0f, 10.0f, 10.0f);
    glUniform3fv(glGetUniformLocation(shader.shader_program, "lightPos"), 1, glm::value_ptr(light_pos));
    glUniform3fv(glGetUniformLocation(shader.shader_program, "viewPos"), 1, glm::value_ptr(camera.position));
    // glUniform4f(glGetUniformLocation(shader.shader_program, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);

    // textures (multi-sampling)
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasTexture"), texture_id != 0);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasRoughness"), roughness_id != 0);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_hasMetallic"), metallic_id != 0);

    // 0 - diffuse
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id); // If 0, it unbinds
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_diffuseMap"), 0);

    // 1 - roughness
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, roughness_id);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_roughnessMap"), 1);

    // 2 - metallic
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallic_id);
    glUniform1i(glGetUniformLocation(shader.shader_program, "u_metallicMap"), 2);

    // rendering wireframe
    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
    } else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(VAO);
    for (const auto& obj : objects)
    {
        if (obj.vertex_count == 0) continue;

        glm::mat4 model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, this->position + obj.position);

        glm::vec3 total_rot = this->rotation + obj.rotation;
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.x), glm::vec3(1, 0, 0));
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.y), glm::vec3(0, 1, 0));
        model_matrix = glm::rotate(model_matrix, glm::radians(total_rot.z), glm::vec3(0, 0, 1));
        model_matrix = glm::scale(model_matrix, glm::vec3(this->scale * obj.scale));

        glUniformMatrix4fv(glGetUniformLocation(shader.shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));

        glDrawArrays(GL_TRIANGLES, obj.vertex_offset, obj.vertex_count);
    }

    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void VortexModel::parse_mtl(const std::string& mtl_filepath)
{
    size_t last_slash = mtl_filepath.find_last_of("/\\");
    std::string mtl_dir = (last_slash == std::string::npos) ? "" : mtl_filepath.substr(0, last_slash + 1);

    std::ifstream file(mtl_filepath);
    if (!file.is_open())
    {
        std::cerr << "[MTL ERROR] Failed to open: " << mtl_filepath << std::endl;
        return;
    }

    std::cout << "[MTL] Reading material: " << mtl_filepath << std::endl;

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
            std::cout << "      + Diffuse Map found: " << tex_filename << std::endl;
            texture_id = load_texture(full_tex_path.c_str());
        }
        else if (prefix == "map_Ns")
        {
            std::string rough_filename;
            ss >> rough_filename;

            std::string full_rough_path = mtl_dir + rough_filename;
            std::cout << "      + Roughness Map found: " << rough_filename << std::endl;
            roughness_id = load_texture(full_rough_path.c_str());
        }
        else if (prefix == "map_refl")
        {
            std::string metallic_filename;
            ss >> metallic_filename;

            std::string full_metallic_path = mtl_dir + metallic_filename;
            std::cout << "      + Metallic Map found: " << metallic_filename << std::endl;
            metallic_id = load_texture(full_metallic_path.c_str());
        }
        else if (prefix == "map_Bump")
        {
            std::string normal_filename;
            while(ss >> normal_filename);

            std::string full_normal_path = mtl_dir + normal_filename;
            std::cout << "      + Normal Map found: " << normal_filename << std::endl;
            normal_id = load_texture(full_normal_path.c_str());
        }
    }
}

unsigned int VortexModel::load_texture(const std::string& path)
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

        std::cout << "[TEXTURE] Successfully loaded: " << path 
                  << " (" << width << "x" << height << ", "
                  << nrComponents << " channels)" << std::endl;

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "[TEXTURE ERROR] Failed to load: " << path << std::endl;
        std::cerr << "               Reason: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void VortexModel::load_obj(const std::string& path)
{
    min_y = 1e10f;
    max_y = -1e10f;

    size_t last_slash = path.find_last_of("/\\");
    std::string obj_dir = (last_slash == std::string::npos) ? "" : path.substr(0, last_slash + 1);
    std::string filename = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);

    size_t last_dot = filename.find_last_of(".");
    if (last_dot == std::string::npos)
    {
        model_name = filename;
    }
    else
    {
        model_name = filename.substr(0, last_dot);
    }

    std::cout << "-------------------------------------------------------" << std::endl;
    std::cout << "[MODEL] Loading: " << path << std::endl;

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_tex_coords;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[MODEL ERROR] Could not open OBJ file: " << path << std::endl;
        if (!std::filesystem::exists(path))
        {
            std::cerr << "              Check: File does not exist at this path." << std::endl;
        }
        return;
    }

    VortexModel_Object *current_obj = nullptr;

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
            if (check_file.is_open())
            {
                check_file.close();
                parse_mtl(path1);
            }
            else
            {
                parse_mtl(path2);
            }
        }
        else if (prefix == "o")
        {
            if (current_obj != nullptr)
            {
                current_obj->vertex_count = (int)vertices.size() - current_obj->vertex_offset;
            }

            VortexModel_Object new_obj;
            ss >> new_obj.name;

            new_obj.vertex_offset = (int)vertices.size();
            new_obj.vertex_count = 0;
            new_obj.position = glm::vec3(0.0f);
            new_obj.rotation = glm::vec3(0.0f);
            new_obj.scale = glm::vec3(1.0f);

            objects.push_back(new_obj);
            current_obj = &objects.back();
        }
        else if (prefix == "v")
        {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            temp_positions.push_back(v);

            if (v.y < min_y) min_y = v.y;
            if (v.y > max_y) max_y = v.y;
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
            if (current_obj == nullptr)
            {
                VortexModel_Object default_obj;
                default_obj.name = "default";
                default_obj.vertex_offset = 0;
                objects.push_back(default_obj);
                current_obj = &objects.back();
            }

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

                VortexModel_Vertex v;
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

    if (current_obj != nullptr)
    {
        current_obj->vertex_count = (int)vertices.size() - current_obj->vertex_offset;
    }

    model_height = max_y - min_y;

    for (auto &obj : objects)
    {
        if (obj.vertex_count == 0) continue;

        glm::vec3 min_p(1e10f), max_p(-1e10f);
        glm::vec3 centroid(0.0f);
        for (int i = 0; i < obj.vertex_count; i++)
        {
            glm::vec3 p = vertices[obj.vertex_offset + i].position;
            centroid += p;

            if (p.x < min_p.x) min_p.x = p.x;
            if (p.y < min_p.y) min_p.y = p.y;
            if (p.z < min_p.z) min_p.z = p.z;
            if (p.x > max_p.x) max_p.x = p.x;
            if (p.y > max_p.y) max_p.y = p.y;
            if (p.z > max_p.z) max_p.z = p.z;
        }
        centroid /= (float)obj.vertex_count;

        for (int i = 0; i < obj.vertex_count; i++)
        {
            vertices[obj.vertex_offset + i].position -= centroid;
        }

        obj.position = centroid;
        obj.b_min = min_p - centroid;
        obj.b_max = max_p - centroid;
    }
}

void VortexModel::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VortexModel_Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position (Location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)0);

    // Normal (Location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)offsetof(VortexModel_Vertex, normal));

    // TexCoords (Location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)offsetof(VortexModel_Vertex, tex_coords));

    glBindVertexArray(0);
}

std::vector<VortexModel_Object>& VortexModel::get_objects()
{
    return objects;
}

bool check_collision(const VortexModel_Object &a, const VortexModel_Object &b)
{
    bool x_axis = a.get_world_min().x <= b.get_world_max().x && a.get_world_max().x >= b.get_world_min().x;
    bool y_axis = a.get_world_min().y <= b.get_world_max().y && a.get_world_max().y >= b.get_world_min().y;
    bool z_axis = a.get_world_min().z <= b.get_world_max().z && a.get_world_max().z >= b.get_world_min().z;

    return x_axis && y_axis && z_axis;
}

VortexModel::~VortexModel()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
}
