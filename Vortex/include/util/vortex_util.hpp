#pragma once

#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <sstream>
#include <iomanip>
#include <filesystem>

struct VortexModelDetailsDict
{
    std::string name;
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 scale;
};

inline void vortex_strncpy(char *dest, size_t dest_size, const char *src)
{
    if (!dest || dest_size == 0) return;

    #ifdef _WIN32
        strncpy_s(dest, dest_size, src, _TRUNCATE);
    #else
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    #endif
}

inline std::string vortex_floatstring(float value, int precision = 2)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

inline std::string vortex_quatstring(const glm::quat &quat_)
{
    return std::string("(") +
        "W:" + vortex_floatstring(quat_.w) + ", " +
        "X:" + vortex_floatstring(quat_.x) + ", " +
        "Y:" + vortex_floatstring(quat_.y) + ", " +
        "Z:" + vortex_floatstring(quat_.z) +
    ")";
}

inline std::string vortex_vec3string(const glm::vec3 &vec3_)
{
    return std::string("(") +
        "X:" + vortex_floatstring(vec3_.x) + ", " +
        "Y:" + vortex_floatstring(vec3_.y) + ", " +
        "Z:" + vortex_floatstring(vec3_.z) +
    ")";
}

inline std::string vortex_ModelDetailsDictToString(const VortexModelDetailsDict &model_details_dict)
{
    std::string str_pos = vortex_vec3string(model_details_dict.position);
    std::string str_orient = vortex_quatstring(model_details_dict.orientation);
    std::string str_scale = vortex_vec3string(model_details_dict.scale);

    return std::string("{\n\t") +
        "    Name: " + model_details_dict.name + "\n\t" +
        "    Pos: " + str_pos + "\n\t" +
        "    Rot: " + str_orient + "\n\t" +
        "    Scale: " + str_scale +
    "\n  }";
}

template<typename... Args>
inline std::string vortex_generatepath(const Args&... parts)
{
    std::filesystem::path path;

    ((path /= parts), ...);

    return path.string();
}
