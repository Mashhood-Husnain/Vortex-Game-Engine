#pragma once
#include "vortex_hud.hpp"
#include <string>
#include <vector>
#include <imgui.h>

enum class VortexCanvas_UIType { TEXT, BAR, COUNTER, PANEL };


struct VortexCanvas_UIElement
{
    std::string id;
    VortexCanvas_UIType type;
    
    // Absolute or anchored screen coordinates for dragging
    ImVec2 position = ImVec2(0.0f, 0.0f);
    ImVec2 size = ImVec2(100.0f, 20.0f);
    
    std::string text_data = "New Text";
    float current_value = 0.0f;
    float max_value = 100.0f;
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    bool is_active = true;
};

class VortexCanvas
{
public:
    std::string canvas_name;
    bool is_active = true;
    std::vector<VortexCanvas_UIElement> elements;

    VortexCanvas(const std::string& name) : canvas_name(name) {}

    void add_element(const VortexCanvas_UIElement& element);

    VortexCanvas_UIElement* get_element(const std::string& element_id);

    void draw();
};
