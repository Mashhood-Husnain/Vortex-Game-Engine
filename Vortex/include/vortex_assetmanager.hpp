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

struct VortexModel_Object;
struct VortexModel_Vertex;

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

class VortexAssetManager
{
    static std::unordered_map<std::string, SharedMesh*> mesh_vault;
public:
    static std::unordered_map<std::string, int> spawn_counts;
    
    static SharedMesh* get_mesh(const std::string &filepath);
    static void clean_up();
};
