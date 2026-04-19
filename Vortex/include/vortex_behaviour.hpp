#pragma once


#include <string>
#include <json.hpp>

class VortexModel;

using json = nlohmann::json;

class VortexMonoBehaviour
{
public:
    VortexModel *gameObject=nullptr;

    virtual void on_start() {}
    virtual void on_update(float deltaTime) {}
    virtual void late_update(float deltaTime) {}

    virtual ~VortexMonoBehaviour() = default;

    virtual void serialize(json &j) {}
    virtual void deserialize(const json &j) {}

    virtual void on_message(const std::string& message, void* data = nullptr) {}
};
