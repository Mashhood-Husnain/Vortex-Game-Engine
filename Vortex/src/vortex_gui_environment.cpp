#include "vortex_gui.hpp"
#include "vortex_application.hpp"

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

void VortexGUI::gui_set_post_processor()
{
    if (m_selected_shader_idx == 0)
    {
        app->set_post_processor(nullptr);
    }
    else
    {
        std::string fragPath = "shaders/" + m_shader_files[m_selected_shader_idx];
        app->set_post_processor(new PostProcessor("shaders/post_process.vert", fragPath));
    }
}

void VortexGUI::post_process_options()
{
    if (!show_gui) return;
    if (!m_shaders_loaded) refresh_shader_list();

    ImGui::SetNextWindowPos(ImVec2(180, 105), ImGuiCond_FirstUseEver);
    ImGui::Begin("Post-Processing", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::Combo("Effect", &m_selected_shader_idx, VortexGuiLambda::DataGetter, static_cast<void*>(&m_display_names), static_cast<int>(m_display_names.size())))
    {
        gui_set_post_processor();
    }

    if (ImGui::Button("Refresh Shaders")) m_shaders_loaded = false;

    ImGui::End();
}

void VortexGUI::refresh_skybox_list()
{
    m_skybox_display_names.clear();
    for (auto &skybox_path : m_skybox_files)
    {
        skybox_path.clear();
    }
    m_skybox_files.clear();

    m_skybox_display_names.push_back("None (Standard)");
    m_skybox_files.push_back({});

    std::vector<std::string> found_skyboxes = get_skyboxes("assets/images/skybox");
    for (std::string name : found_skyboxes)
    {
        m_skybox_display_names.push_back(name);

        std::vector<std::string> skybox_path = {
            // Order: right, left bottom, top, front, back
            "assets/images/skybox/" + name + "_right.png",
            "assets/images/skybox/" + name + "_left.png",
            "assets/images/skybox/" + name + "_top.png",
            "assets/images/skybox/" + name + "_bottom.png",
            "assets/images/skybox/" + name + "_front.png",
            "assets/images/skybox/" + name + "_back.png",
        };

        m_skybox_files.push_back(skybox_path);
    }

    m_skybox_loaded = true;
}

void VortexGUI::gui_set_skybox()
{
    if (m_selected_skybox_idx == 0)
    {
        app->set_skybox(nullptr);
    }
    else
    {
        app->set_skybox(new VortexSkybox(m_skybox_files[m_selected_skybox_idx]));
    }
}

void VortexGUI::skybox_options()
{   
    if (!show_gui) return;
    if (!m_skybox_loaded) refresh_skybox_list();

    ImGui::SetNextWindowPos(ImVec2(10, 190), ImGuiCond_FirstUseEver);
    ImGui::Begin(
        "SkyBox",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse
    );

    if (m_skybox_display_names.size() <= 1)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No skyboxes found in assets/");
    }

    else
    {
        if (ImGui::Combo("Environment", &m_selected_skybox_idx, VortexGuiLambda::DataGetter, static_cast<void*>(&m_skybox_display_names), static_cast<int>(m_skybox_display_names.size())))
        {
            gui_set_skybox();
        }
    }

    if (ImGui::Button("Refresh Skyboxes")) m_skybox_loaded = false;

    ImGui::End();
}
