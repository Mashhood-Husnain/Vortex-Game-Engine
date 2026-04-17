#include "vortex_rigidbody.hpp"

void VortexRigidbody::on_update(float deltaTime)
{
    if (is_kinematic) return;

    velocity.y -= gravity * deltaTime;

    gameObject->transform.position.y += velocity.y * deltaTime;
    if (check_world_collision())
    {
        gameObject->transform.position.y -= velocity.y * deltaTime;
        if (velocity.y <= 0) is_grounded = true;
        velocity.y = 0.0f;
    }
    else
    {
        is_grounded = false;
    }

    gameObject->transform.position.x += velocity.x * deltaTime;
    if (check_world_collision())
    {
        gameObject->transform.position.x -= velocity.x * deltaTime;
        velocity.x = 0.0f;
    }

    gameObject->transform.position.z += velocity.z * deltaTime;
    if (check_world_collision())
    {
        gameObject->transform.position.z -= velocity.z * deltaTime;
        velocity.z = 0.0f;
    }
}

bool VortexRigidbody::check_world_collision()
{
    for (VortexModel* other : VortexObjectManager::active_models)
    {
        if (other == gameObject || !other->is_active) continue;

        CollisionHit hit = VortexPhysics::check_collision_detailed(gameObject, other);
        if (hit.has_hit)
        {
            return true;
        }
    }
    return false;
}

VORTEX_REGISTER_SCRIPT(VortexRigidbody);
