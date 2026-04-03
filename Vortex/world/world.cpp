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
#include "util/vortex_global_vars.hpp"

#include "vortex_skybox.hpp"

int main() {
    VortexCamera camera(glm::vec3(15.0f, 2.0f, 1.0f));
    camera.look_at(glm::vec3(0.0f, 2.0f, 0.0f));

    VortexWindow window("Vortex Engine", &camera, 900, 600);

    VortexShader default_shader("shaders/default.vert", "shaders/default.frag");
    VortexShader particle_shader("shaders/particles.vert", "shaders/particles.frag");

    // VortexModel cube("assets/models/obj/cube.obj", &window);
    VortexModel ground("assets/models/obj/flat_plane.obj", &window);
    // VortexModel house("assets/models/obj/abandoned_house.obj", &window);

    ParticleSystem ps(1000000, &window, "Default");

    window.run([&](){
        // cube.draw(default_shader, camera, window.show_wireframe);
        // house.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
        
        if (!window.shadow_manager->is_active)
        {
            // ps_smoke(glm::vec3(0, 0, 0), ps, particle_shader, camera);
            ps_orbit(glm::vec3(0, 2, 0), ps, particle_shader, camera);
        }
    });

    return 0;
}
