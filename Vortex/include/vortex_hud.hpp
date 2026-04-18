#pragma once
#include <string>
#include <imgui.h>
#include <glm/glm.hpp>

#include "vortex_camera.hpp"

enum class UIAnchor
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

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
    static void Text(const char* format, ...);
    static ImVec2 GetAnchorPosition(UIAnchor anchor, float offset_x = 0.0f, float offset_y = 0.0f);
    static ImVec2 GetScreenPercent(float percent_x, float percent_y);

    static glm::vec2 GetScreenDimensions();
};
