#include "vortex_rigidbody.hpp"

void VortexRigidbody::on_update()
{
    if (is_kinematic) return;

    if (gravity) velocity.y -= gravity_value * GLOBAL::deltaTime;

    int micro_steps = 10;

    float y_move = velocity.y * GLOBAL::deltaTime;
    transform().position.y += y_move;
    object().set_model_matrix(object().get_model_matrix());

    if (check_world_collision())
    {
        transform().position.y -= y_move;

        float step = y_move / static_cast<float>(micro_steps);
        for (int i = 0; i < micro_steps; i++)
        {
            transform().position.y += step;
            object().set_model_matrix(object().get_model_matrix());

            if (check_world_collision())
            {
                transform().position.y -= step;
                object().set_model_matrix(object().get_model_matrix());
                break;
            }
        }

        if (velocity.y <= 0) is_grounded = true;
        velocity.y = 0.0f;
    }
    else
    {
        is_grounded = false;
    }

    float x_move = velocity.x * GLOBAL::deltaTime;
    transform().position.x += x_move;
    object().set_model_matrix(object().get_model_matrix());

    if (check_world_collision())
    {
        transform().position.x -= x_move;

        float step = x_move / static_cast<float>(micro_steps);
        for (int i = 0; i < micro_steps; i++)
        {
            transform().position.x += step;
            object().set_model_matrix(object().get_model_matrix());
            if (check_world_collision())
            {
                transform().position.x -= step;
                object().set_model_matrix(object().get_model_matrix());
                break;
            }
        }
        velocity.x = 0.0f;
    }

    float z_move = velocity.z * GLOBAL::deltaTime;
    transform().position.z += z_move;
    object().set_model_matrix(object().get_model_matrix());

    if (check_world_collision())
    {
        transform().position.z -= z_move;

        float step = z_move / static_cast<float>(micro_steps);
        for (int i = 0; i < micro_steps; i++)
        {
            transform().position.z += step;
            object().set_model_matrix(object().get_model_matrix());
            if (check_world_collision())
            {
                transform().position.z -= step;
                object().set_model_matrix(object().get_model_matrix());
                break;
            }
        }
        velocity.z = 0.0f;
    }
}

bool VortexRigidbody::check_world_collision()
{
    if (std::abs(velocity.x) < 0.001f && std::abs(velocity.y) < 0.001f && std::abs(velocity.z) < 0.001f)
    {
        return false;
    }

    const glm::vec3& this_pos = transform().position;

    for (VortexModel* other : VortexObjectManager::active_models)
    {
        if (other == &object() || !other->is_active) continue;

        if (!other->rigidbody) continue;

        bool is_other_dynamic = (other->rigidbody && !other->rigidbody->is_kinematic);

        if (is_other_dynamic)
        {
            const glm::vec3 delta = other->transform.position - this_pos;
            float distance_sq = glm::dot(delta, delta);

            if (distance_sq > 400.0f) continue;
        }

        CollisionHit hit = VortexPhysics::check_collision_detailed(&object(), other);
        if (hit.has_hit)
        {
            return true;
        }
    }

    return false;
}

void VortexRigidbody::serialize(json& j)
{
    j["is_kinematic"] = is_kinematic;
    j["gravity"] = gravity;
    j["gravity_value"] = gravity_value;
}

void VortexRigidbody::deserialize(const json& j)
{
    if (j.contains("is_kinematic"))
    {
        is_kinematic = j["is_kinematic"];
    }
    if (j.contains("gravity"))
    {
        gravity = j["gravity"];
    }
    if (j.contains("gravity_value"))
    {
        gravity_value = j["gravity_value"];
    }
}
