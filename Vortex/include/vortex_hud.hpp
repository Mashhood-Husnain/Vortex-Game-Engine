#pragma once
#include <string>
#include <imgui.h>
#include <glm/glm.hpp>

#include "vortex_camera.hpp"

class VortexHUD {
    static bool ProjectToScreen(glm::vec3 world_pos, VortexCamera* camera, ImVec2& out_screen_pos);
public:
    static void Begin();
    static void End();

    static void Label(const std::string& text, ImVec2 pos, ImVec4 color = ImVec4(1, 1, 1, 1));
    static void Bar(const std::string& label, float current, float max, ImVec2 pos, ImVec2 size, ImVec4 color);
    static void Counter(const std::string& label, int value, ImVec2 pos, ImVec4 color = ImVec4(1, 1, 0, 1));
    static void Rect(ImVec2 pos, ImVec2 size, ImVec4 color, float thickness = 1.0f);
    static void WorldLabel(const std::string& text, glm::vec3 world_pos, VortexCamera* camera, ImVec4 color = ImVec4(1, 1, 1, 1));
    static void WorldBar(float current, float max, glm::vec3 world_pos, VortexCamera* camera, ImVec2 size = ImVec2(100, 10), ImVec4 color = ImVec4(1, 0, 0, 1));

    static glm::vec2 GetScreenDimensions();
};
