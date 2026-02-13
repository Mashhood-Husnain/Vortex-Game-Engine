#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_objreader.hpp"

int main() {

    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    OpenGLShader default_shader("shaders/default.vert", "shaders/default.frag");

    // OpenGLModel cube("models/disfigured_cube.obj");
    OpenGLModel plane("models/flat_plane.obj");
    OpenGLModel drawer("models/drawer.obj");

    window.run([&](){
        // cube.draw(default_shader, camera);

        drawer.draw(default_shader, camera);
        plane.draw(default_shader, camera);

    });

    return 0;
}
