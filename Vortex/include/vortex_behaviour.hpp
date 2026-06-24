#pragma once

#include <string>
#include <json.hpp>
#include <glm/glm.hpp>

class VortexApplication;
class VortexModel;
struct Transform;

using json = nlohmann::json;

enum class ScriptVarType { INT, FLOAT, BOOL, STRING, VEC3 };

struct ScriptVariable {
    std::string name;
    ScriptVarType type;
    void *data_ptr;
    bool show_in_editor;
};

class VortexMonoBehaviour
{
    friend class VortexModel;
public:

    Transform *vortexTransform = nullptr;
    VortexApplication *vortexEngine = nullptr;
    VortexModel *vortexGameObject = nullptr;

    Transform &transform()
    {
        return *vortexTransform;
    }

    VortexApplication &engine()
    {
        return *vortexEngine;
    }

    VortexModel &object()
    {
        return *vortexGameObject;
    }

    std::vector<ScriptVariable> exposed_variables;

    void expose_int(const std::string &name, int *ptr, const bool &show_in_editor)
    {
        exposed_variables.push_back({name, ScriptVarType::INT, ptr, show_in_editor});
    }

    void expose_float(const std::string &name, float *ptr, const bool &show_in_editor)
    {
        exposed_variables.push_back({name, ScriptVarType::FLOAT, ptr, show_in_editor});
    }

    void expose_bool(const std::string &name, bool *ptr, const bool &show_in_editor)
    {
        exposed_variables.push_back({name, ScriptVarType::BOOL, ptr, show_in_editor});
    }

    void expose_string(const std::string &name, std::string *ptr, const bool &show_in_editor)
    {
        exposed_variables.push_back({name, ScriptVarType::STRING, ptr, show_in_editor});
    }

    void expose_vec3(const std::string &name, glm::vec3 *ptr, const bool &show_in_editor)
    {
        exposed_variables.push_back({name, ScriptVarType::VEC3, ptr, show_in_editor});
    }

    template <typename T>
    T* search_variable_by_name(const std::string &name, ScriptVarType expected_type)
    {
        for (auto &var : exposed_variables)
        {
            if (var.name == name && var.type == expected_type)
            {
                return static_cast<T*> (var.data_ptr);
            }
        }
        return nullptr;
    }

    virtual void on_start() {}
    virtual void on_update() {}
    virtual void late_update() {}
    virtual void on_destroy() {}

    virtual ~VortexMonoBehaviour() = default;

    virtual void serialize(json &j)
    {
        for (const auto &var : exposed_variables)
        {
            if (var.type == ScriptVarType::INT) j[var.name] = *(int*) var.data_ptr;
            else if (var.type == ScriptVarType::FLOAT) j[var.name] = *(float*) var.data_ptr;
            else if (var.type == ScriptVarType::BOOL) j[var.name] = *(bool*) var.data_ptr;
            else if (var.type == ScriptVarType::STRING) j[var.name] = *(std::string*) var.data_ptr;
            else if (var.type == ScriptVarType::VEC3)
            {
                glm::vec3 *v = (glm::vec3*) var.data_ptr;
                j[var.name] = {v->x, v->y, v->z};
            }
        }
    }

    virtual void deserialize(const json &j)
    {
        for (const auto &var : exposed_variables)
        {
            if (var.type == ScriptVarType::INT) *(int*) var.data_ptr = j[var.name];
            else if (var.type == ScriptVarType::FLOAT) *(float*) var.data_ptr = j[var.name];
            else if (var.type == ScriptVarType::BOOL) *(bool*) var.data_ptr = j[var.name];
            else if (var.type == ScriptVarType::STRING) *(std::string*) var.data_ptr = j[var.name];
            else if (var.type == ScriptVarType::VEC3)
            {
                glm::vec3 *v = (glm::vec3*) var.data_ptr;
                auto &arr = j[var.name];
                v->x = arr[0];
                v->y = arr[1];
                v->z = arr[2];
            }
        }
    }

    virtual void on_message(const std::string& message, void* data = nullptr) {}
};
