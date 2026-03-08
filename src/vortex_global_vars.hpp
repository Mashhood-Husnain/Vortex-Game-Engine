#pragma once

#define global_WORLDGRAVITY 9.80f

#define COLOR_SMOKE glm::vec4(0.7f, 0.7f, 0.7f, 0.4f)
#define COLOR_BROWN glm::vec4(0.3f, 0.2f, 0.1f, 1.0f)
#define COLOR_ORANGE glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)

static const float global_DEFAULTPARTICLEVERTICES[] = {
    -0.05f,  0.05f, 0.0f,
    -0.05f, -0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,

    -0.05f,  0.05f, 0.0f,
     0.05f, -0.05f, 0.0f,
     0.05f,  0.05f, 0.0f
};

static const float global_WORLDAXESVERTICES[] = {
    0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // X-axis (Red)
    5.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,

    0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // Y-axis (Green)
    0.0f, 5.0f, 0.0f,   0.0f, 1.0f, 0.0f,

    0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // Z-axis (Blue)
    0.0f, 0.0f, 5.0f,   0.0f, 0.0f, 1.0f
};
