#include "vortex_gui.hpp"
#include "vortex_model.hpp"

VortexGUI::VortexGUI()
{

}

void VortexGUI::init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.IniFilename = nullptr;

    ImGuiStyle &style = ImGui::GetStyle();

    style.WindowRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;

    ImVec4 *colors = style.Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void VortexGUI::update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_processed_models.clear();
}

void VortexGUI::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void VortexGUI::show_engine_stats()
{
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);

    ImGui::Begin("Performance");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::End();
}

void VortexGUI::begin_scene_inspector()
{
    ImGui::SetNextWindowPos(ImVec2(10, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 50), ImVec2(FLT_MAX, 500));
    
    ImGui::Begin("Scene Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
}

void VortexGUI::show_model_info(VortexModel* model)
{
    if (!model) return;

    if (m_processed_models.find(model) != m_processed_models.end()) return;

    m_processed_models.insert(model);

    std::string header_id = model->model_name + "##" + std::to_string((uintptr_t)model);

    if (ImGui::CollapsingHeader(header_id.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID(model);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

        ImGui::Spacing();
        ImGui::SeparatorText("Model Info");

        ImGui::Text("Objects: %zu", model->objects.size());

        ImGui::Spacing();
        ImGui::SeparatorText("Transform");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 8));

        if (ImGui::BeginTable("TransformTable", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            ImGui::Text("Position");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);            
            ImGui::DragFloat3("##pos", &model->position.x, 0.1f);

            ImGui::TableNextColumn();
            ImGui::Text("Scale");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat3("##scale", &model->scale.x, 0.01f, 0.001f, 10.0f);

            ImGui::TableNextColumn();
            ImGui::Text("Rotation");
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat3("##rot", &model->rotation.x, 0.1f);

            ImGui::EndTable();
        }

        ImGui::Spacing();
        ImGui::SeparatorText("Scaling");

        float uniform_scale = model->scale.x;
        if (ImGui::SliderFloat("Uniform Scale", &uniform_scale, 0.001f, 10.0f))
        {
            model->scale = glm::vec3(uniform_scale);
        }

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

        if (ImGui::Button("Reset Transform", ImVec2(-1, 0)))
        {
            model->position = glm::vec3(0.0f);
            model->scale    = glm::vec3(1.0f);
            model->rotation = glm::vec3(0.0f);
        }

        ImGui::PopStyleColor(2);

        ImGui::PopStyleVar(3);
        ImGui::PopID();

        ImGui::Spacing();
    }
}

void VortexGUI::end_scene_inspector()
{
    ImGui::End();
}

VortexGUI::~VortexGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
