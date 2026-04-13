/*
 * File: vortex_skybox.cpp
 * Project: VortexEngine
 * Description: Cubemap Skybox implementation
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_skybox.hpp"

VortexSkybox::VortexSkybox(std::vector<std::string> faces)
{
    shader = new VortexShader("shaders/skybox.vert", "shaders/skybox.frag");

    setup_mesh();

    cubemapTexture = load_cubemap(faces);

    shader->use();
    shader->setInt("skybox", 0);
}

VortexSkybox::~VortexSkybox()
{
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteTextures(1, &cubemapTexture);

    delete shader;
    shader = nullptr;
}

void VortexSkybox::draw(VortexCamera *camera)
{
    glDepthFunc(GL_LEQUAL);

    shader->use();
    shader->setMat4("view", camera->getViewMatrix());
    shader->setMat4("projection", camera->getProjectionMatrix());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

unsigned int VortexSkybox::load_cubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;

    stbi_set_flip_vertically_on_load(false);
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        std::cout << "[SKYBOX] Loading: " << faces[i];

        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            std::cout << " [SUCCESS]" << std::endl; 
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                format, width, height, 0, format,
                GL_UNSIGNED_BYTE, data
            );

            stbi_image_free(data);
        }
        else
        {
            std::cout << " [FAILED]" << std::endl;
            std::cout << "[SKYBOX ERROR] Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);

            exit(EXIT_FAILURE);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void VortexSkybox::setup_mesh()
{
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLOBAL::DEFAULT_VERTICES::SKYBOX_VERTICES), &GLOBAL::DEFAULT_VERTICES::SKYBOX_VERTICES, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

std::vector<std::string> get_skyboxes(std::string path)
{
    std::vector<std::string> names;

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        std::cout << "[SKYBOX ERROR] Path specified does not exist: " << path << std::endl;
        exit(EXIT_FAILURE);
    }

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
            std::string filename = entry.path().filename().string();

            size_t pos = filename.find("_right");

            if (pos != std::string::npos)
            {
                std::string name = filename.substr(0, pos);
                names.push_back(name);
            }
        }
    }
    
    return names;
}
