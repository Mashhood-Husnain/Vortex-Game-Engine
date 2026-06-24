#include "vortex_decal.hpp"
#include "vortex_application.hpp"
#include "vortex_objectmanager.hpp"

void VortexDecal::on_start()
{
    quad_model = new VortexModel("assets/models/obj/flat_plane.obj", &engine());
    quad_model->is_active = false;
    scale = glm::vec3(0.1f, 1.0f, 0.1f);
}

void VortexDecal::set_texture(const std::string &path)
{
    if (!quad_model) return;

    if (!std::filesystem::exists(path))
    {
        VORTEX_WARN("[DECAL] Texture file does not exist: ", path);
        VortexAssetManager::release_texture(decal_texture_path);
        decal_texture_path = "";
        quad_model->texture_id = 0;
        return;
    }

    VortexAssetManager::release_texture(decal_texture_path);
    decal_texture_path = path;
    quad_model->texture_id = VortexAssetManager::load_texture_raw(path);
}

void VortexDecal::draw(VortexCamera* camera, bool use_scene_lights)
{
    if (!quad_model || decal_texture_path.empty()) return;

    glm::mat4 parent_world = glm::translate(glm::mat4(1.0f), vortexTransform->position) *
                             glm::mat4_cast(vortexTransform->orientation) *
                             glm::scale(glm::mat4(1.0f), vortexTransform->scale);

    glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotation_matrix = glm::mat4_cast(orientation);

    float final_scale_x = scale.x * (flip_x ? -1.0f : 1.0f);
    float final_scale_z = scale.z * (flip_y ? -1.0f : 1.0f);

    glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(final_scale_x, 1.0f, final_scale_z));

    glm::mat4 local_matrix = translation_matrix * rotation_matrix * scale_matrix;
    glm::mat4 final_matrix = parent_world * local_matrix;

    quad_model->set_model_matrix(final_matrix);

    VortexShader* d_shader = VortexObjectManager::get_decal_shader();
    d_shader->use();
    d_shader->setMat4("view", camera->getViewMatrix());
    d_shader->setMat4("projection", camera->getProjectionMatrix());
    d_shader->setMat4("model", final_matrix);

    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(final_matrix)));
    d_shader->setMat3("normalMatrix", normal_matrix);
    d_shader->setVec3("viewPos", camera->get_position());

    VortexModel::bind_lights_to_shader(d_shader, use_scene_lights);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quad_model->texture_id);
    d_shader->setInt("u_diffuseMap", 0);
    d_shader->setVec2("u_textureScale", glm::vec2(1.0f, 1.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glBindVertexArray(quad_model->shared_data->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void VortexDecal::serialize(json &j)
{
    j["texture_path"] = decal_texture_path;
    j["pos_x"] = position.x; j["pos_y"] = position.y; j["pos_z"] = position.z;
    j["rot_w"] = orientation.w; j["rot_x"] = orientation.x; j["rot_y"] = orientation.y; j["rot_z"] = orientation.z;
    j["scale_x"] = scale.x; j["scale_y"] = scale.y; j["scale_z"] = scale.z;
    j["flip_x"] = flip_x; j["flip_y"] = flip_y;
}

void VortexDecal::deserialize(const json &j)
{
    if (!quad_model) on_start();

    if (j.contains("texture_path"))
    {
        std::string loaded_path = j["texture_path"];
        if (!loaded_path.empty()) set_texture(loaded_path);
    }

    if (j.contains("pos_x")) position = glm::vec3(j["pos_x"], j["pos_y"], j["pos_z"]);
    if (j.contains("rot_w")) orientation = glm::quat(j["rot_w"], j["rot_x"], j["rot_y"], j["rot_z"]);
    if (j.contains("scale_x")) scale = glm::vec3(j["scale_x"], j["scale_y"], j["scale_z"]);
    if (j.contains("flip_x")) flip_x = j["flip_x"];
    if (j.contains("flip_y")) flip_y = j["flip_y"];
}

VortexDecal::~VortexDecal()
{
    VortexAssetManager::release_texture(decal_texture_path);
    if (quad_model) delete quad_model;
}