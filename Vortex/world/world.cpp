/*
 * File: world.cpp
 * Project: VortexEngine
 * Description: Running the engine
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_window.hpp"
#include "vortex_camera.hpp"

int main() {
    VortexCamera camera(glm::vec3(15.0f, 2.0f, 1.0f));
    camera.look_at(glm::vec3(0.0f, 2.0f, 0.0f));

    VortexWindow window("Vortex Engine", &camera, 900, 600);
    
    window.run([&](){});

    return 0;
}
