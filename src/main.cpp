#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_stlreader.hpp"

int main() {

    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    openGLShader default_shader("shaders/default.vert", "shaders/default.frag");

    STLModel cube("models/disfigured_cube.stl");

    window.run([&](){

        cube.rotation.y += 50.0f * window.deltaTime;

        cube.draw(default_shader, camera);
    });

    return 0;
}
