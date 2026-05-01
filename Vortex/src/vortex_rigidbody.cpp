#include "vortex_rigidbody.hpp"

void VortexRigidbody::on_update(float deltaTime)
{
    if (is_kinematic) return;

    if (gravity) velocity.y -= gravity_value * deltaTime;

    transform().position.y += velocity.y * deltaTime;
    if (check_world_collision())
    {
        transform().position.y -= velocity.y * deltaTime;
        if (velocity.y <= 0) is_grounded = true;
        velocity.y = 0.0f;
    }
    else
    {
        is_grounded = false;
    }

    transform().position.x += velocity.x * deltaTime;
    if (check_world_collision())
    {
        transform().position.x -= velocity.x * deltaTime;
        velocity.x = 0.0f;
    }

    transform().position.z += velocity.z * deltaTime;
    if (check_world_collision())
    {
        transform().position.z -= velocity.z * deltaTime;
        velocity.z = 0.0f;
    }
}

bool VortexRigidbody::check_world_collision()
{
    for (VortexModel* other : VortexObjectManager::active_models)
    {
        if (other == &object() || !other->is_active) continue;

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
