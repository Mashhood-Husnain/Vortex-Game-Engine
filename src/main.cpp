#include "opengl_window.hpp"
#include "opengl_shaders.hpp"

void draw_rectangle(const openGLShader& shader, unsigned int* VAO)
{
    glUseProgram(shader.shader_program);

    int vertexColorLocation = glGetUniformLocation(shader.shader_program, "u_Color");
    glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f);

    glBindVertexArray(*VAO);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main() {

    OpenGLWindow window("OpenGL Window");    
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
        draw_rectangle(rectangle_shader, &VAO);
    });

    return 0;
}
