#include "vortex_window.hpp"
#include "vortex_shaders.hpp"
#include "vortex_camera.hpp"
#include "vortex_objreader.hpp"
#include "vortex_player.hpp"
#include "vortex_particlesystem.hpp"

#include "vortex_ps_test.hpp"

int main() {
    VortexCamera camera(glm::vec3(-5.0f, 20.0f, 0.0f));
    camera.look_at(glm::vec3(10.0f, 0.0f, 0.0f));

    VortexWindow window("Vortex Window", &camera);

    VortexShader default_shader("shaders/default.vert", "shaders/default.frag");
    VortexShader particle_shader("shaders/particles.vert", "shaders/particles.frag");

    ParticleSystem particle_system(1000);

    VortexModel ground("assets/models/obj/flat_plane.obj");

    srand(static_cast<unsigned int>(time(0))); // initalize random seed

    window.run([&](){
        float dt = window.deltaTime;

        if (glfwGetKey(window.get_window_ptr(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            // ps_explosion(glm::vec3(0, 1, 0), particle_system, 250);
            ps_circle(glm::vec3(0, 1, 0), particle_system, 50);
        }

        ground.draw(default_shader, camera, window.show_wireframe);
        
        particle_system.update(dt);
        particle_system.draw(particle_shader, camera);
    });

    return 0;
}
