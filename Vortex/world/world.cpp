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

    VortexWindow window("Vortex Window", &camera);

    VortexShader default_shader("shaders/default.vert", "shaders/default.frag");
    VortexShader particle_shader("shaders/particles.vert", "shaders/particles.frag");

    ParticleSystem particle_system(10000);

    VortexModel ground("assets/models/obj/flat_plane.obj", &window);
    VortexModel player_body("assets/models/obj/player.obj", &window);
    VortexModel house("assets/models/obj/abandoned_house.obj", &window);
    VortexModel cube("assets/models/obj/cube.obj", &window);

    Player player("player", &camera, &player_body, &default_shader);
    
    cube.position = glm::vec3(10.0f, 1.0f, 10.0f);
    ground.scale *= 5.0f;
    player_body.scale *= 0.7f;

    align_on_top(cube, ground);
    // align_on_top(house, ground);
    align_on_top(player_body, ground);

    window.run([&](){

        float time = glfwGetTime();
        float radius = 10.0f;
        float height = 10.0f;
        float speed = 0.5f;

        GLOBAL::LIGHTPOS.x = sin(time * speed) * radius;
        GLOBAL::LIGHTPOS.y = height;
        GLOBAL::LIGHTPOS.z = cos(time * speed) * radius;

        cube.draw(default_shader, camera, window.show_wireframe);
        house.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
        player.update(&window);
        
        if (!window.shadow_manager->is_active)
        {
            float dt = window.deltaTime;

            ps_smoke(glm::vec3(10, 0, 0), particle_system, 1);
            ps_smoke(glm::vec3(11, 0, 0), particle_system, 1);
            ps_smoke(glm::vec3(9, 0, 0), particle_system, 1);
            ps_smoke(glm::vec3(10, 0, 1), particle_system, 1);
            ps_smoke(glm::vec3(10, 0, -1), particle_system, 1);

            particle_system.update(dt);
            particle_system.draw(particle_shader, camera);
        }
    });

    return 0;
}
