#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"
#include "opengl_stlreader.hpp"

int main() {

    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    openGLShader default_shader("shaders/default.vert", "shaders/default.frag");

    STLModel cube("models/cube.stl");

    window.run([&](){
        cube.draw(default_shader, camera);
    });

    return 0;
}
