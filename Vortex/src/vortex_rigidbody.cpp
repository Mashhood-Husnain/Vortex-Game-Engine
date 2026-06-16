#include "vortex_rigidbody.hpp"

void VortexRigidbody::on_update()
{
    if (is_kinematic) return;

    if (gravity) velocity.y -= gravity_value * GLOBAL::deltaTime;

    transform().position.y += velocity.y * GLOBAL::deltaTime;
    if (check_world_collision())
    {
        transform().position.y -= velocity.y * GLOBAL::deltaTime;
        if (velocity.y <= 0) is_grounded = true;
        velocity.y = 0.0f;
    }
    else
    {
        is_grounded = false;
    }

    transform().position.x += velocity.x * GLOBAL::deltaTime;
    if (check_world_collision())
    {
        transform().position.x -= velocity.x * GLOBAL::deltaTime;
        velocity.x = 0.0f;
    }

    transform().position.z += velocity.z * GLOBAL::deltaTime;
    if (check_world_collision())
    {
        transform().position.z -= velocity.z * GLOBAL::deltaTime;
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
}
