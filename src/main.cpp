#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_objreader.hpp"

int main() {

    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    OpenGLShader default_shader("shaders/default.vert", "shaders/default.frag");

    // OpenGLModel cube("models/disfigured_cube.obj");
    OpenGLModel plane("models/obj/flat_plane.obj");
    // OpenGLModel drawer("models/drawer.obj");

    // plane.scale = 5.0f;
    // cube.position.x += 5.0f;

    // OpenGLModel environ("models/test_environment.obj");
    OpenGLModel house("models/obj/abandoned_house.obj");

    window.run([&](){
        // cube.draw(default_shader, camera);
        // drawer.draw(default_shader, camera, window.show_wireframe);
        plane.draw(default_shader, camera, window.show_wireframe);

        // environ.draw(default_shader, camera, window.show_wireframe);
        house.draw(default_shader, camera, window.show_wireframe);
    });

    return 0;
}
