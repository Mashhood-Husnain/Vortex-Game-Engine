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
#include "vortex_postprocessor.hpp"
#include "test/vortex_ps_test.hpp"
#include "util/vortex_global_vars.hpp"

int main() {
    VortexCamera camera(glm::vec3(15.0f, 2.0f, 1.0f));
    camera.look_at(glm::vec3(0.0f, 2.0f, 0.0f));

    VortexWindow window("Vortex Engine", &camera);

    VortexShader default_shader("shaders/default.vert", "shaders/default.frag");

    VortexModel ground("assets/models/obj/flat_plane.obj", &window);
    // VortexModel cube("assets/models/obj/cube.obj", &window);
    VortexModel house("assets/models/obj/abandoned_house.obj", &window);

    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_analog.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_night_vision.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_retro_ps1.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_thermal.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_blueprint.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_vhs_glitch.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_dot_matrix.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_blur.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_drunk.frag");
    // PostProcessor post_process("shaders/post_process.vert", "shaders/post_process_8bit.frag");

    // window.set_post_processor(&post_process);

    window.run([&](){
        // cube.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
        house.draw(default_shader, camera, window.show_wireframe);
        
        if (!window.shadow_manager->is_active)
        {
            
        }
    });

    return 0;
}
