#include "vortex_assetmanager.hpp"

std::unordered_map<std::string, SharedMesh*> VortexAssetManager::mesh_vault;

static unsigned int load_texture_helper(const std::string& path, SharedMesh* mesh)
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "[TEXTURE] Successfully loaded: " << path << std::endl;
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "[TEXTURE ERROR] Failed to load: " << path << std::endl;
        stbi_image_free(data);
        exit(EXIT_FAILURE);
    }

    mesh->m_gpu_allocated_textures.push_back(textureID);
    return textureID;
}

static void parse_mtl_helper(const std::string& mtl_filepath, SharedMesh* mesh)
{
    size_t last_slash = mtl_filepath.find_last_of("/\\");
    std::string mtl_dir = (last_slash == std::string::npos) ? "" : mtl_filepath.substr(0, last_slash + 1);

    std::ifstream file(mtl_filepath);
    if (!file.is_open()) return;

    std::cout << "[MTL] Reading material: " << mtl_filepath << std::endl;
    std::string line;
    while(std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "map_Kd")
        {
            std::string tex_filename; ss >> tex_filename;
            mesh->texture_id = load_texture_helper(mtl_dir + tex_filename, mesh);
        }
        else if (prefix == "map_Ns")
        {
            std::string rough_filename; ss >> rough_filename;
            mesh->roughness_id = load_texture_helper(mtl_dir + rough_filename, mesh);
        }
        else if (prefix == "map_refl")
        {
            std::string metallic_filename; ss >> metallic_filename;
            mesh->metallic_id = load_texture_helper(mtl_dir + metallic_filename, mesh);
        }
        else if (prefix == "map_Bump")
        {
            std::string normal_filename; while(ss >> normal_filename);
            mesh->normal_id = load_texture_helper(mtl_dir + normal_filename, mesh);
        }
    }
}

SharedMesh* VortexAssetManager::get_mesh(const std::string &filepath)
{
    if (mesh_vault.find(filepath) != mesh_vault.end())
    {
        return mesh_vault[filepath];
    }

    std::cout << "[ASSET MANAGER] First time loading: " << filepath << std::endl;

    SharedMesh *new_mesh = new SharedMesh();
    std::vector<VortexModel_Vertex> vertices;

    float min_y = 1e10f, max_y = -1e10f;
    size_t last_slash = filepath.find_last_of("/\\");
    std::string obj_dir = (last_slash == std::string::npos) ? "" : filepath.substr(0, last_slash + 1);

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_tex_coords;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(filepath);
    if (!file.is_open()) exit(EXIT_FAILURE);

    VortexModel_Object *current_obj = nullptr;
    std::string line;
    
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if(prefix == "mtllib")
        {
            std::string mtl_filename; ss >> mtl_filename;
            std::string path1 = obj_dir + mtl_filename;            
            std::string path2 = obj_dir + "../mtl/" + mtl_filename;
            std::ifstream check_file(path1);
            if (check_file.is_open())
            {
                check_file.close(); parse_mtl_helper(path1, new_mesh);
            }
            else
            {
                parse_mtl_helper(path2, new_mesh);
            }
        }
        else if (prefix == "o")
        {
            if (current_obj != nullptr) current_obj->vertex_count = (int)vertices.size() - current_obj->vertex_offset;
            VortexModel_Object new_obj;
            ss >> new_obj.name;
            new_obj.vertex_offset = (int)vertices.size();
            new_obj.vertex_count = 0;
            new_obj.transform.position = glm::vec3(0.0f);
            new_obj.transform.rotation = glm::vec3(0.0f);
            new_obj.transform.scale = glm::vec3(1.0f);
            new_mesh->objects.push_back(new_obj);
            current_obj = &new_mesh->objects.back();
        }
        else if (prefix == "v")
        {
            glm::vec3 v; ss >> v.x >> v.y >> v.z;
            temp_positions.push_back(v);
            if (v.y < min_y) min_y = v.y;
            if (v.y > max_y) max_y = v.y;
        } 
        else if (prefix == "vt")
        {
            glm::vec2 vt; ss >> vt.x >> vt.y;
            temp_tex_coords.push_back(vt);
        } 
        else if (prefix == "vn")
        {
            glm::vec3 vn; ss >> vn.x >> vn.y >> vn.z;
            temp_normals.push_back(vn);
        } 
        else if (prefix == "f")
        {
            if (current_obj == nullptr)
            {
                VortexModel_Object default_obj;
                default_obj.name = "default";
                default_obj.vertex_offset = 0;
                new_mesh->objects.push_back(default_obj);
                current_obj = &new_mesh->objects.back();
            }

            for (int i = 0; i < 3; i++)
            {
                std::string vertexData; ss >> vertexData;
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
                v.tex_coords = (vtIdx > 0) ? temp_tex_coords[vtIdx - 1] : glm::vec2(0.0f);
                v.normal = (vnIdx > 0) ? temp_normals[vnIdx - 1] : glm::vec3(0.0f, 1.0f, 0.0f);
                vertices.push_back(v);
            }
        }
    }

    if (current_obj != nullptr) current_obj->vertex_count = (int)vertices.size() - current_obj->vertex_offset;
    new_mesh->model_height = max_y - min_y;

    for (auto &obj : new_mesh->objects)
    {
        if (obj.vertex_count == 0) continue;
        glm::vec3 min_p(1e10f), max_p(-1e10f), centroid(0.0f);
        
        for (int i = 0; i < obj.vertex_count; i++)
        {
            glm::vec3 p = vertices[obj.vertex_offset + i].position;
            centroid += p;
            if (p.x < min_p.x) min_p.x = p.x; if (p.y < min_p.y) min_p.y = p.y; if (p.z < min_p.z) min_p.z = p.z;
            if (p.x > max_p.x) max_p.x = p.x; if (p.y > max_p.y) max_p.y = p.y; if (p.z > max_p.z) max_p.z = p.z;
        }
        centroid /= (float)obj.vertex_count;

        for (int i = 0; i < obj.vertex_count; i++)
        {
            vertices[obj.vertex_offset + i].position.x -= centroid.x;
            vertices[obj.vertex_offset + i].position.z -= centroid.z;
            vertices[obj.vertex_offset + i].position.y -= min_p.y; 
        }

        obj.transform.position = glm::vec3(centroid.x, min_p.y, centroid.z);
        obj.b_min = min_p - obj.transform.position;
        obj.b_max = max_p - obj.transform.position;
    }

    glGenVertexArrays(1, &new_mesh->VAO);
    glGenBuffers(1, &new_mesh->VBO);
    glBindVertexArray(new_mesh->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, new_mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VortexModel_Vertex), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)offsetof(VortexModel_Vertex, normal));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VortexModel_Vertex), (void*)offsetof(VortexModel_Vertex, tex_coords));
    glBindVertexArray(0);

    mesh_vault[filepath] = new_mesh;
    
    std::cout << "[ASSET MANAGER] Success! Cached in VRAM." << std::endl;
    return new_mesh;
}

void VortexAssetManager::clean_up()
{
    for (auto const& [path, mesh] : mesh_vault)
    {
        if (mesh->VAO != 0) glDeleteVertexArrays(1, &mesh->VAO);
        if (mesh->VBO != 0) glDeleteBuffers(1, &mesh->VBO);

        if (mesh->texture_id != 0) glDeleteTextures(1, &mesh->texture_id);
        if (mesh->roughness_id != 0) glDeleteTextures(1, &mesh->roughness_id);
        if (mesh->metallic_id != 0) glDeleteTextures(1, &mesh->metallic_id);
        if (mesh->normal_id != 0) glDeleteTextures(1, &mesh->normal_id);

        for (unsigned int tex_id : mesh->m_gpu_allocated_textures)
        {
            glDeleteTextures(1, &tex_id);
        }
        delete mesh;
    }
    mesh_vault.clear();
}

