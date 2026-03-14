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

    ImGui::StyleColorsDark();
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

void VortexGUI::show_model_info(VortexModel *model)
{
    if (!model) return;

    if (m_processed_models.find(model) != m_processed_models.end())
    {
        return; 
    }

    m_processed_models.insert(model);

    std::string header_id = model->model_name + "##" + std::to_string((uintptr_t)model);

    if (ImGui::CollapsingHeader(header_id.c_str()))
    {
        ImGui::PushID(model);
        ImGui::Indent();
        
        ImGui::Text("Objects: %zu", model->objects.size());
        ImGui::DragFloat3("Pos", &model->position.x, 0.1f);
        ImGui::DragFloat3("Scale", &model->scale.x, 0.01f, 0.001f, 10.0f);

        float currentScale = model->scale.x;
        if (ImGui::DragFloat("Uniform Scale", &currentScale, 0.01f, 0.001f, 10.0f))
        {
            model->scale = glm::vec3(currentScale);
        }

        if (ImGui::Button("Reset")) {
            model->position = glm::vec3(0.0f);
            model->scale = glm::vec3(1.0f);
        }
        
        ImGui::Unindent();
        ImGui::Spacing();

        ImGui::PopID();
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
