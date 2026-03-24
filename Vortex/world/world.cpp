/*
 * File: world.cpp
 * Project: VortexEngine
 * Description: Running the engine
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_window.hpp"
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_model.hpp"
#include "vortex_player.hpp"
#include "vortex_particlesystem.hpp"

#include "test/vortex_ps_test.hpp"

int main() {
    VortexCamera camera(glm::vec3(0.0f, 2.0f, 5.0f));
    camera.look_at(glm::vec3(0.0f, 2.0f, 0.0f));

    VortexWindow window("Vortex Engine", &camera);

    VortexShader default_shader("shaders/default.vert", "shaders/default.frag");

    VortexModel ground("assets/models/obj/flat_plane.obj", &window);
    VortexModel cube("assets/models/obj/cube.obj", &window);


    window.run([&](){
        cube.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
        
        if (!window.shadow_manager->is_active)
        {
            
        }
    });

    return 0;
}
