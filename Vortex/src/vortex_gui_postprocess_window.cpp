#include "vortex_gui_ImGui.hpp"


PostProcessWindow::PostProcessWindow() : VortexImGuiWindow(getGuiWindowTypeName(GuiWindowTypes::PostProcessWindow), true) {}

void PostProcessWindow::apply_window_constraints()
{

}

void PostProcessWindow::refresh_shader_list()
{

}

void PostProcessWindow::set_post_post_processor()
{
    
}

void PostProcessWindow::draw_content()
{
    if (!show_options) return;
    if (!m_shaders_loaded) refresh_shader_list();

    if (ImGui::CollapsingHeader("Screen Effects", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Spacing();
        ImGui::TextDisabled("Active Screen Effect");

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::Combo("##Effect", &m_selected_shader_idx, VortexUI::Lambda::DataGetter, static_cast<void*>(&m_display_names), static_cast<int>(m_display_names.size())))
        {
            set_post_post_processor();
        }

        ImGui::Spacing();

        if (ImGui::Button("Refresh Shaders", ImVec2(-1, 28)))
        {
            m_shaders_loaded = false;
        }

        ImGui::Spacing();
    }
}
