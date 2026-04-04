#pragma once

#include <glad/glad.h>
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"

class VortexGrid {
    unsigned int VAO, VBO;
    VortexShader* shader;

public:
    VortexGrid();
    ~VortexGrid();
    void draw(VortexCamera& camera);
};
