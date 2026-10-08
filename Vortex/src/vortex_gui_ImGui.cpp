#include "vortex_gui_ImGui.hpp"

VortexDecal *VortexInspectorWindow::g_active_decal_for_gizmo = nullptr;

VortexImGuiWindow::VortexImGuiWindow(const std::string &name, bool default_open)
{
    window_name = name;
    is_open = default_open;
}

void VortexImGuiWindow::apply_window_constraints() {}

void VortexImGuiWindow::render()
{
    if (!is_open) return;

    apply_window_constraints();

    if (ImGui::Begin(window_name.c_str(), &is_open, get_window_flags()))
    {
        draw_content();
    }

    ImGui::End();
}

ImGuiWindowFlags VortexImGuiWindow::get_window_flags() const
{
    return ImGuiWindowFlags_None;
}

void VortexUI::LabeledText(const char *label, const char *text_value, float offset)
{
    ImGui::TextDisabled("%s", label);
    ImGui::SameLine(offset);
    ImGui::TextUnformatted(text_value);
}

void VortexUI::InputTextLine(const char *label, char *buffer, size_t buffer_size, std::function<void(const std::string&)> on_change)
{
    ImGui::TextDisabled("%s", label);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputText((std::string("##") + label).c_str(), buffer, buffer_size))
    {
        if (on_change) on_change(std::string(buffer));
    }
}

void VortexUI::DragDropTargetBox(const char *payload_id, const char *hint_text, std::function<void(const char*)> on_drop, float height)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.3f, 0.4f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

    ImGui::BeginChild((std::string("##DropZone") + hint_text).c_str(), ImVec2(0, height), true);
    ImVec2 text_size = ImGui::CalcTextSize(hint_text);
    ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - text_size.x) * 0.5f, (ImGui::GetWindowHeight() - text_size.y) * 0.5f));
    ImGui::TextColored(ImVec4(0.5f, 0.7f, 1.0f, 1.0f), "%s", hint_text);
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payload_id))
        {
            if (on_drop) on_drop(static_cast<const char*>(payload->Data));
        }
        ImGui::EndDragDropTarget();
    }
}

bool VortexUI::Helpers::DrawHeader(const char *label, bool is_selected)
{
    if (is_selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.35f, 0.45f, 0.60f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.30f, 0.40f, 0.50f, 0.80f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.35f, 0.45f, 0.55f, 1.00f));
    }

    bool open = ImGui::CollapsingHeader(label);

    if (is_selected) ImGui::PopStyleColor(3);

    return open;
}

bool VortexUI::Helpers::DrawComponentNode(const char *label, int flags, const ImVec4 &colors)
{
    ImGui::PushStyleColor(ImGuiCol_Header, colors);

    bool open = ImGui::TreeNodeEx(label, flags);
    ImGui::PopStyleColor();

    return open;
}

bool VortexUI::Helpers::DrawDangerButton(const char *label, const ImVec2 &size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.60f, 0.20f, 0.20f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.70f, 0.25f, 0.25f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.80f, 0.30f, 0.30f, 1.00f));

    bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);

    return pressed;
}

bool VortexUI::Helpers::DrawActionButton(const char *label, const ImVec2 &size)
{
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.40f, 0.60f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.45f, 0.70f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.50f, 0.80f, 1.00f));

    bool pressed = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);

    return pressed;
}

void VortexUI::Helpers::DrawTransformRow(const char *label, const char *id, float *data, float step, float min, float max)
{
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", label);

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-FLT_MIN);

    if (min != max) ImGui::DragFloat3(id, data, step, min, max);
    else ImGui::DragFloat3(id, data, step);
}

void VortexUI::Lambda::ClampedInputFloat(const char *label, float *v, float step, float step_fast, float v_min, float v_max, const char *format)
{
    if (ImGui::InputFloat(label, v, step, step_fast, format))
    {
        *v = std::clamp(*v, v_min, v_max);
    }
}

void VortexUI::Lambda::ClampedInputInt(const char *label, int *v, int step, int step_fast, int v_min, int v_max)
{
    if (ImGui::InputInt(label, v, step, step_fast))
    {
        *v = std::clamp(*v, v_min, v_max);
    }
}

const char *VortexUI::Lambda::DataGetter(void *data, int idx)
{
    auto *items = static_cast<std::vector<std::string>*>(data);

    if (idx < 0 || idx >= static_cast<int>(items->size()))
    {
        return nullptr;
    }

    return (*items)[idx].c_str();
}
