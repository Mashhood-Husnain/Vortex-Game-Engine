#pragma once

#include "vortex_model.hpp"

class VortexModel;

class VortexMonoBehaviour
{
public:
    VortexModel *gameObject=nullptr;
    std::map<std::string, float> *blackboard = nullptr;

    virtual void on_start() {}
    virtual void on_update(float deltaTime) {}
    virtual void late_update(float deltaTime) {}

    virtual ~VortexMonoBehaviour() = default;

    void VortexMonoBehaviour_set_value(std::string key, float val)
    {
        if (blackboard)
        {
            (*blackboard)[key] = val;
        }
    }

    float VortexMonoBehaviour_get_value(std::string key)
    {
        if (blackboard && blackboard->count(key))
        {
            return (*blackboard)[key];
        }

        return 0.0f;
    }
};
