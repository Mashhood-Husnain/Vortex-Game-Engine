#include "opengl_shaders.hpp"

bool openGLShader::read_shader(std::string& vertex_path, std::string& fragment_path)
{
    std::ifstream vertex_file(vertex_path);
    std::ifstream fragment_file(fragment_path);

    if (!vertex_file.is_open() || !fragment_file.is_open())
    {
        std::cerr << "ERROR: could not open shader files!" << std::endl;
        return false;
    }

    std::stringstream vertex_stream, fragment_stream;

    vertex_stream << vertex_file.rdbuf();
    fragment_stream << fragment_file.rdbuf();

    vertex_file.close();
    fragment_file.close();

    vertex_shader_code = vertex_stream.str();
    fragment_shader_code = fragment_stream.str();

    return true;
}

void openGLShader::init_shader()
{
    const char* vertex_shader_source = vertex_shader_code.c_str();
    const char* fragment_shader_source = fragment_shader_code.c_str();

    // compile
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);

    // link into program
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    // delete shaders
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

openGLShader::openGLShader(std::string vertex_path, std::string fragment_path)
{
    if (!read_shader(vertex_path, fragment_path)) exit(EXIT_FAILURE);;
    init_shader();
}
