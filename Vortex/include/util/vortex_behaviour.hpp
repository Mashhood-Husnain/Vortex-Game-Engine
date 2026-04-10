#pragma once

#include "vortex_model.hpp"

class VortexModel;

class VortexMonoBehaviour
{
public:
    VortexModel *gameObject=nullptr;

    virtual void on_start() {};
    virtual void on_update(float deltaTime) {};

    virtual ~VortexMonoBehaviour() = default;
};

