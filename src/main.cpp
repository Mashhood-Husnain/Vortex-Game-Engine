#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_objreader.hpp"
#include "player.hpp"
#include "particle_system.hpp"

#include <cstdlib>

void trigger_explosion(glm::vec3 position, ParticleSystem& ps)
{
    float debris_size = 0.5f;
    float debris_lifetime = 2.0f;
    float debris_gravity = 1.0f;
    float debris_drag = 0.1f;
    float smoke_size = 0.5f;
    float smoke_lifetime = 2.5f;
    float smoke_gravity = -0.1f;
    float smoke_drag = 0.8f;

    for (int i = 0; i < 250; i++)
    {
        glm::vec3 debris_vel(
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100 - 50) / 10.0f
        );
        ps.emit(
            position,
            debris_size,
            debris_vel,
            debris_lifetime,
            debris_gravity,
            debris_drag,
            COLOR_BROWN
        );

        float offsetX = (rand() % 100 - 50) / 100.0f;
        float offsetY = (rand() % 100 - 50) / 100.0f;
        float offsetZ = (rand() % 100 - 50) / 100.0f;
        glm::vec3 position_rand = position + glm::vec3(offsetX, offsetY, offsetZ);

        glm::vec3 smoke_vel(
            (rand() % 100 -50) / 40.0f,
            (rand() % 100 -50) / 20.0f,
            (rand() % 100 -50) / 40.0f
        );
        ps.emit(
            position_rand,
            smoke_size,
            smoke_vel,
            smoke_lifetime,
            smoke_gravity,
            smoke_drag,
            COLOR_SMOKE
        );
    }
}

int main() {
    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);
    OpenGLShader default_shader("shaders/default.vert", "shaders/default.frag");
    OpenGLShader particle_shader("shaders/particles.vert", "shaders/particles.frag");

    // OpenGLModel house_model("models/obj/abandoned_house.obj");
    // OpenGLModel player_model("models/obj/player.obj");

    // Player player("Mashhood", &camera, &player_model, &default_shader);
    // player_model.scale = glm::vec3(0.7f);
   
    // OpenGLModel drawer("models/obj/drawer.obj");
    // OpenGLModel ground("models/obj/ground.obj");    
    // drawer.position = glm::vec3(0.0f, 0.0f, -5.0f);
    // drawer.scale = glm::vec3(0.2f);
    // ground.scale = glm::vec3(3.0f, 3.0f, 3.0f);

    // float total_time = 0.0f;
    // window.run([&](){
    //     // // temp fog experiment
    //     // glUseProgram(default_shader.shader_program);
    //     // glUniform3f(glGetUniformLocation(default_shader.shader_program, "u_fogColor"), window.bg_color.r, window.bg_color.g, window.bg_color.b);
    //     // glUniform1f(glGetUniformLocation(default_shader.shader_program, "u_fogDensity"), 0.10f);

    //     // drawer.draw(default_shader, camera, window.show_wireframe);
    //     // ground.draw(default_shader, camera, window.show_wireframe);

    //     total_time += window.deltaTime;
    //     std::vector<OpenGLModel_Object>& house_parts = house_model.get_objects();

    //     for (size_t i = 0; i <house_parts.size(); i++)
    //     {
    //         OpenGLModel_Object& part = house_parts[i];

    //         float movement = sin(total_time * 2.0f + i) * 0.5f;
    //         float pulse = 1.0f + sin(total_time * 3.0f + i) * 0.2f;

    //         part.position.x += movement * window.deltaTime;
    //         part.position.y -= movement * window.deltaTime;
    //         part.position.z += movement * window.deltaTime;
    //         part.scale = glm::vec3(pulse);
    //     }

    //     player.update(window.get_window_ptr(), window.deltaTime);
    //     house_model.draw(default_shader, camera, window.show_wireframe);
    // });

    int particles_to_render = 1000;
    ParticleSystem particle_system(particles_to_render);
    srand(static_cast<unsigned int>(time(0))); // initalize random seed

    OpenGLModel ground("models/obj/flat_plane.obj");
    OpenGLModel player_body("models/obj/player.obj");

    Player player("Mashhood", &camera, &player_body, &default_shader);

    window.run([&](){
        float dt = window.deltaTime;

        if (glfwGetKey(window.get_window_ptr(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            trigger_explosion(glm::vec3(0, 1, 0), particle_system);
        }

        ground.draw(default_shader, camera, window.show_wireframe);
        
        particle_system.update(dt);
        particle_system.draw(particle_shader, camera);

        player.update(window.get_window_ptr(), window.deltaTime);
    });

    return 0;
}
