#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_objreader.hpp"
#include "player.hpp"


int main() {
    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    OpenGLShader default_shader("shaders/default.vert", "shaders/default.frag");
   
    OpenGLModel drawer("models/obj/drawer.obj");
    OpenGLModel house("models/obj/abandoned_house.obj");
    OpenGLModel player_model("models/obj/player.obj");
    OpenGLModel ground("models/obj/ground.obj");

    Player player("Mashhood", &camera, &player_model, &default_shader);
    
    drawer.position = glm::vec3(0.0f, 0.0f, -5.0f);
    drawer.scale = glm::vec3(0.2f);

    player_model.scale = glm::vec3(0.7f);
    ground.scale = glm::vec3(3.0f, 3.0f, 3.0f);

    window.run([&](){
        player.update(window.get_window_ptr(), window.deltaTime);
        drawer.draw(default_shader, camera, window.show_wireframe);
        house.draw(default_shader, camera, window.show_wireframe);
        ground.draw(default_shader, camera, window.show_wireframe);
    });

    return 0;
}
