#pragma once

#include <glad/glad.h>
#include "glm/glm.hpp"
#include <vector>

class VortexCamera;
class VortexShader;
class VortexModel;

class VortexBoxCollider
{
    unsigned VAO = 0, VBO = 0, EBO = 0;
public:
    glm::vec3 min;
    glm::vec3 max;

    bool show_collider_box = true;

    VortexBoxCollider();
    ~VortexBoxCollider();

    void setup_visual_mesh();
    void draw(VortexShader &collider_shader, VortexCamera &camera, VortexModel *model);
};
