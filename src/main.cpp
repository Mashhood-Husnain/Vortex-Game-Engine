#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_objreader.hpp"
#include "player.hpp"
#include "particle_system.hpp"

#include <cstdlib>

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

    ParticleSystem ps(500);
    srand(static_cast<unsigned int>(time(0)));

    float spawn_timer = 0.0f;

    window.run([&](){
        float dt = window.deltaTime;
        spawn_timer += dt;

        if (spawn_timer > 0.1f)
        {
            for (int i = 0; i < 5; i++)
            {
                glm::vec3 origin(0.0f, 2.0f, 0.0f);

                glm::vec3 velocity(
                    (rand() % 100 - 50) / 25.0f,
                    (rand() % 100) / 20.0f,
                    (rand() % 100 - 50) / 25.0f
                );

                float life = 1.0f + (rand() % 100 / 100.0f);
                ps.emit(origin, velocity, life);
            }
            spawn_timer = 0.0f;
        }

        ps.update(dt);
        ps.draw(particle_shader, camera);
    });

    return 0;
}
