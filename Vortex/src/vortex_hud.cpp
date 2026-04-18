#include "vortex_hud.hpp"

void VortexHUD::Begin()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | 
                            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Vortex_HUD_Canvas", nullptr, flags);
}

void VortexHUD::End()
{
    ImGui::End();
}

void VortexHUD::Label(const std::string& text, ImVec2 pos, ImVec4 color)
{
    ImGui::SetCursorPos(pos);
    ImGui::TextColored(color, "%s", text.c_str());
}

void VortexHUD::Bar(const std::string& label, float current, float max, ImVec2 pos, ImVec2 size, ImVec4 color)
{
    ImGui::SetCursorPos(pos);
    
    if (!label.empty())
    {
        ImGui::Text("%s", label.c_str());
        ImGui::SetCursorPos(ImVec2(pos.x, pos.y + 20));
    }

    float fraction = (max > 0) ? (current / max) : 0.0f;
    
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(fraction, size, "");
    ImGui::PopStyleColor();
}

void VortexHUD::Counter(const std::string& label, int value, ImVec2 pos, ImVec4 color)
{
    ImGui::SetCursorPos(pos);
    ImGui::TextColored(color, "%s: %d", label.c_str(), value);
}

void VortexHUD::Rect(ImVec2 pos, ImVec2 size, ImVec4 color, float thickness)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p_min = pos;
    ImVec2 p_max = ImVec2(pos.x + size.x, pos.y + size.y);

    draw_list->AddRect(p_min, p_max, ImColor(color), 0.0f, 0, thickness);
}

bool VortexHUD::ProjectToScreen(glm::vec3 world_pos, VortexCamera* camera, ImVec2& out_screen_pos)
{
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();
    
    glm::vec4 clip_space_pos = projection * view * glm::vec4(world_pos, 1.0f);

    if (clip_space_pos.w <= 0.001f) return false;

    glm::vec3 ndc = glm::vec3(clip_space_pos) / clip_space_pos.w;

    ImGuiIO& io = ImGui::GetIO();
    out_screen_pos.x = (ndc.x + 1.0f) * 0.5f * io.DisplaySize.x;
    out_screen_pos.y = (1.0f - ndc.y) * 0.5f * io.DisplaySize.y;

    return true;
}

void VortexHUD::WorldLabel(const std::string& text, glm::vec3 world_pos, VortexCamera* camera, ImVec4 color)
{
    ImVec2 screen_pos;
    
    if (ProjectToScreen(world_pos, camera, screen_pos))
    {
        float text_width = ImGui::CalcTextSize(text.c_str()).x;
        Label(text, ImVec2(screen_pos.x - (text_width * 0.5f), screen_pos.y), color);
    }
}

void VortexHUD::WorldBar(float current, float max, glm::vec3 world_pos, VortexCamera* camera, ImVec2 size, ImVec4 color)
{
    ImVec2 screen_pos;
    
    if (ProjectToScreen(world_pos, camera, screen_pos))
    {
        Bar("", current, max, ImVec2(screen_pos.x - (size.x * 0.5f), screen_pos.y), size, color);
    }
}

glm::vec2 VortexHUD::GetScreenDimensions()
{
    ImGuiIO& io = ImGui::GetIO();
    return glm::vec2(io.DisplaySize.x, io.DisplaySize.y);
}

void VortexHUD::Text(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    ImGui::TextV(format, args);

    va_end(args);
}


ImVec2 VortexHUD::GetAnchorPosition(UIAnchor anchor, float offset_x, float offset_y)
{
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    ImVec2 base_pos(0.0f, 0.0f);

    switch (anchor)
    {
        case UIAnchor::TOP_LEFT:      base_pos = ImVec2(0.0f, 0.0f); break;
        case UIAnchor::TOP_CENTER:    base_pos = ImVec2(screen_size.x * 0.5f, 0.0f); break;
        case UIAnchor::TOP_RIGHT:     base_pos = ImVec2(screen_size.x, 0.0f); break;
        
        case UIAnchor::CENTER_LEFT:   base_pos = ImVec2(0.0f, screen_size.y * 0.5f); break;
        case UIAnchor::CENTER:        base_pos = ImVec2(screen_size.x * 0.5f, screen_size.y * 0.5f); break;
        case UIAnchor::CENTER_RIGHT:  base_pos = ImVec2(screen_size.x, screen_size.y * 0.5f); break;
        
        case UIAnchor::BOTTOM_LEFT:   base_pos = ImVec2(0.0f, screen_size.y); break;
        case UIAnchor::BOTTOM_CENTER: base_pos = ImVec2(screen_size.x * 0.5f, screen_size.y); break;
        case UIAnchor::BOTTOM_RIGHT:  base_pos = ImVec2(screen_size.x, screen_size.y); break;
    }

    return ImVec2(base_pos.x + offset_x, base_pos.y + offset_y);
}

ImVec2 VortexHUD::GetScreenPercent(float percent_x, float percent_y)
{
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    return ImVec2(screen_size.x * percent_x, screen_size.y * percent_y);
}
