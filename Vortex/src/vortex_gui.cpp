#include "vortex_gui.hpp"
#include "vortex_model.hpp"
#include "vortex_camera.hpp"
#include "vortex_window.hpp"

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

EngineStatsThreaded g_engine_stats;
void VortexGUI::show_engine_stats()
{
    ImGui::SetNextWindowPos(ImVec2(10, 15), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        "Performance", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );

    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f", fps);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / fps);

    ImGui::Separator();

    EngineStats stats = g_engine_stats.get_stats();

    ImGui::ProgressBar(
        stats.cpu_usage / 100.0f, ImVec2(-1, 0),
        ("CPU: " + std::to_string((int)stats.cpu_usage) + "%").c_str()
    );

    ImGui::Text("RAM Usage: %.2f MB", stats.ram_usage);

    ImGui::Separator();

    if (stats.gpu_usage >= 0.0f)
    {
        ImGui::ProgressBar(
            stats.gpu_usage / 100.0f, ImVec2(-1, 0),
            ("GPU: " + std::to_string((int)stats.gpu_usage) + "%").c_str()
        );
    }
    else ImGui::Text("GPU Usage: N/A");

    if (stats.gpu_mem_total > 0)
    {
        ImGui::ProgressBar(
            stats.gpu_mem_usage / stats.gpu_mem_total, ImVec2(-1, 0),
            ("VRAM: " + std::to_string((int)stats.gpu_mem_usage) + " MB").c_str()
        );
    }

    ImGui::End();
}

void VortexGUI::show_camera_info(VortexCamera *camera)
{
    if (!camera) return;

    ImGui::SetNextWindowPos(ImVec2(180, 10), ImGuiCond_FirstUseEver);

    ImGui::Begin(
        "Camera", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );

    ImGui::Text("Position: %.1f, %.1f, %.1f", camera->position.x, camera->position.y, camera->position.z);
    ImGui::Text("Is Anchored: %s", camera->anchored ? "True" : "False");
    
    ImGui::InputFloat("Speed", &camera->movement_speed, 0.5f, 1.0f, "%.2f");

    ImGui::End();
}

void VortexGUI::begin_scene_inspector()
{
    ImGui::SetNextWindowPos(ImVec2(10, 190), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 50), ImVec2(FLT_MAX, 500));
    
    ImGui::Begin("Scene Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
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

void VortexGUI::refresh_shader_list()
{
    m_shader_files.clear();
    m_display_names.clear();

    m_shader_files.push_back(""); 
    m_display_names.push_back("None (Standard)");

    namespace fs = std::filesystem;
    std::string path = "shaders/";

    if (fs::exists(path) && fs::is_directory(path))
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            std::string filename = entry.path().filename().string();
            
            if (filename.rfind("post_process_", 0) == 0 && entry.path().extension() == ".frag")
            {
                m_shader_files.push_back(filename);
                
                std::string display = filename.substr(13);
                size_t lastdot = display.find_last_of(".");
                if (lastdot != std::string::npos) display = display.substr(0, lastdot);
                
                m_display_names.push_back(display);
            }
        }
    }
    m_shaders_loaded = true;
}

void VortexGUI::show_post_process_options(VortexWindow *window)
{
    if (!m_shaders_loaded) refresh_shader_list();

    ImGui::SetNextWindowPos(ImVec2(180, 105), ImGuiCond_FirstUseEver);
    ImGui::Begin("Post-Processing", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    auto getter = [](void* data, int idx) -> const char*
    {
        auto* items = static_cast<std::vector<std::string>*>(data);
        if (idx < 0 || idx >= static_cast<int>(items->size())) return nullptr;
        return (*items)[idx].c_str();
    };

    if (ImGui::Combo("Effect", &m_selected_shader_idx, getter, static_cast<void*>(&m_display_names), static_cast<int>(m_display_names.size())))
    {
        if (m_selected_shader_idx == 0)
        {
            window->set_post_processor(nullptr);
        }
        else
        {
            std::string fragPath = "shaders/" + m_shader_files[m_selected_shader_idx];
            window->set_post_processor(new PostProcessor("shaders/post_process.vert", fragPath));
        }
    }

    if (ImGui::Button("Refresh Shaders")) m_shaders_loaded = false;

    ImGui::End();
}

VortexGUI::~VortexGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
