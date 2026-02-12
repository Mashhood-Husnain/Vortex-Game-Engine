#include "opengl_window.hpp"
#include "opengl_shaders.hpp"
#include "opengl_camera.hpp"

#include <glm/gtc/type_ptr.hpp>

void draw_rectangle(OpenGLCamera& camera, const openGLShader& shader, unsigned int* VAO)
{
    glUseProgram(shader.shader_program);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), camera.aspect_ratio, 0.1f, 100.0f);

    int vertexColorLocation = glGetUniformLocation(shader.shader_program, "u_Color");
    glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f);

    unsigned int modelLoc = glGetUniformLocation(shader.shader_program, "model");
    unsigned int viewLoc = glGetUniformLocation(shader.shader_program, "view");
    unsigned int projLoc = glGetUniformLocation(shader.shader_program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(*VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main() {

    OpenGLCamera camera;
    OpenGLWindow window("OpenGL Window", &camera);    
    openGLShader rectangle_shader("shaders/rectangle_shader.vert", "shaders/rectangle_shader.frag");

    float vertices[] = {
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VAO, EBO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    window.run([&](){
        draw_rectangle(camera, rectangle_shader, &VAO);
    });

    return 0;
}
