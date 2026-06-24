#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <glad/glad.h>
#include "stb_image.h"

#include "vortex_model.hpp"
#include "vortex_collider.hpp"
#include "util/vortex_logs.hpp"

struct VortexModel_Object;
struct VortexModel_Vertex;

class VortexBoxCollider;

struct SharedMesh
{
    unsigned int VAO = 0;
    unsigned int VBO = 0;

    unsigned int texture_id = 0;
    unsigned int roughness_id = 0;
    unsigned int metallic_id = 0;
    unsigned int normal_id = 0;

    float model_height = 0.0f;

    VortexBoxCollider collider;

    std::vector<VortexModel_Object> objects;
    std::vector<unsigned int> m_gpu_allocated_textures;
};

struct TextureData
{
    GLuint id = 0;
    int ref_count = 0;
};

struct MeshData
{
    SharedMesh* mesh_ptr = nullptr;
    int ref_count = 0;
};

class VortexAssetManager
{
    static std::unordered_map<std::string, MeshData> mesh_vault;
    static std::unordered_map<std::string, TextureData> texture_vault;
    static void parse_mtl_helper(const std::string& mtl_filepath, SharedMesh* mesh);
public:
    static std::unordered_map<std::string, int> spawn_counts;

    static SharedMesh* get_mesh(const std::string &filepath);
    static void release_mesh(const std::string& filepath);

    static unsigned int load_texture(const std::string& path, SharedMesh* mesh);
    static unsigned int load_texture_raw(const std::string& path);
    static void release_texture(const std::string& path);

    static void clean_up();
};
