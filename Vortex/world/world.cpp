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
    VortexShader light_shader("shaders/default.vert", "shaders/light.frag");

    ParticleSystem particle_system(10000);

    VortexModel ground("assets/models/obj/flat_plane.obj", &window);
    VortexModel cube("assets/models/obj/cube.obj", &window);
    VortexModel light("assets/models/obj/cube.obj", &window);
    VortexModel drawer("assets/models/obj/drawer.obj", &window);
    VortexModel capsule("assets/models/obj/capsule.obj", &window);

    light.scale = glm::vec3(0.25f);

    window.run([&](){
        float time = glfwGetTime();
        float radius = 10.0f;
        float height = 10.0f;
        float speed = 0.5f;

        GLOBAL::LIGHTPOS.x = sin(time * speed) * radius;
        GLOBAL::LIGHTPOS.y = height;
        GLOBAL::LIGHTPOS.z = cos(time * speed) * radius;

        light.position = GLOBAL::LIGHTPOS;

        cube.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
        drawer.draw(default_shader, camera, window.show_wireframe);
        capsule.draw(default_shader, camera, window.show_wireframe);
        
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

            light.draw(light_shader, camera, window.show_wireframe);
        }
    });

    return 0;
}
