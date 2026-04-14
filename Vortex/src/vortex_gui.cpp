#include "vortex_gui.hpp"
#include "vortex_model.hpp"
#include "vortex_application.hpp"
#include "vortex_camera.hpp"
#include "vortex_application.hpp"
#include "vortex_particlesystem.hpp"
#include "util/vortex_save_load.hpp"
#include "util/vortex_script_registry.hpp"

VortexGUI::VortexGUI()
{

}

void VortexGUI::init(VortexApplication *app)
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

    this->app = app;

    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);

    ImGui_ImplGlfw_InitForOpenGL(app->get_window_ptr(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void VortexGUI::update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_processed_models.clear();
    m_processed_ps.clear();
}

void VortexGUI::render()
{
    if (app->current_state == EngineState::EDITOR)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, 10.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
        
        ImGui::SetNextWindowBgAlpha(0.8f); 

        ImGuiWindowFlags toolbar_flags = 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoNav | 
            ImGuiWindowFlags_NoMove;

        ImGui::Begin("Toolbar", nullptr, toolbar_flags);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        
        if (ImGui::Button("RUN GAME", ImVec2(100, 30)))
        {
            std::cout << "[ENGINE] Entering Play Mode..." << std::endl;
            VortexProject::save_project("temp_playmode_backup", app->dynamic_models, app->dynamic_particlesystems);

            app->current_state = EngineState::PLAY;
            app->show_mouse(false);
            app->show_wireframe = false;
            app->view_world_axis = false;
            show_gui = false;

            for (VortexModel *model : app->dynamic_models)
            {
                for (VortexMonoBehaviour *script : model->behaviours)
                {
                    script->on_start();
                }
            }
        }
        
        ImGui::PopStyleColor(2);
        
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

EngineStatsThreaded g_engine_stats;
void VortexGUI::engine_stats()
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

void VortexGUI::camera_info(VortexCamera *camera)
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
    
    VortexGuiLambda::ClampedInputFloat("Speed", &camera->movement_speed, 0.5f, 1.0f, 0.0f, 100.0f);

    ImGui::End();
}

void VortexGUI::begin_scene_inspector()
{
    if (!show_gui) return;

    ImGui::SetNextWindowPos(ImVec2(10, 275), ImGuiCond_FirstUseEver);

    float app_window_height = ImGui::GetIO().DisplaySize.y;
    float max_height = app_window_height - 275.0f - 10.0f;

    if (max_height < 50.0f) max_height = 50.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 50), ImVec2(FLT_MAX, max_height));

    if (m_force_scene_collapse)
    {
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
        m_force_scene_collapse = false;
    }
    
    ImGui::Begin("Scene Inspector", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    bool is_collapsed = ImGui::IsWindowCollapsed();
    if (!is_collapsed && m_scene_was_collapsed)
    {
        m_force_creator_collapse = true;
    }
    m_scene_was_collapsed = is_collapsed;

    m_scene_pos = ImGui::GetWindowPos();
    m_scene_size = ImGui::GetWindowSize();
}

void VortexGUI::inspector_info(VortexModel* model, ParticleSystem *ps)
{
    if (!show_gui || (!model && !ps)) return;

    if (model)
    {
        if (m_processed_models.find(model) != m_processed_models.end()) return;

        m_processed_models.insert(model);
        
        std::string header_id = model->model_name + "##" + std::to_string((uintptr_t)model);

        if (ImGui::CollapsingHeader(header_id.c_str()))
        {
            ImGui::PushID(model);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

            ImGui::Spacing();
            ImGui::SeparatorText("Model Info");

            ImGui::Text("Objects: %zu", model->shared_data->objects.size());

            ImGui::Spacing();
            ImGui::SeparatorText("Attached Scripts");

            if (model->script_names.empty())
            {
                ImGui::TextDisabled("No scripts attached");
            }
            else
            {
                int script_to_delete = -1;

                for (size_t i = 0; i < model->script_names.size(); i++)
                {
                    ImGui::PushID(static_cast<int>(i));

                    ImGui::Bullet();
                    ImGui::SameLine();
                    ImGui::Text("%s", model->script_names[i].c_str());

                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60);

                    if (ImGui::SmallButton("Remove"))
                    {
                        script_to_delete = static_cast<int>(i);
                    }

                    ImGui::PopID();
                }

                if (script_to_delete != -1)
                {
                    delete model->behaviours[script_to_delete];
                    model->behaviours.erase(model->behaviours.begin() + script_to_delete);
                    model->script_names.erase(model->script_names.begin() + script_to_delete);
                }
            }

            ImGui::Spacing();
            std::vector<std::string> available_scripts = ScriptRegistry::get().get_avaialble_scripts();
            if (ImGui::BeginCombo("Add Script", "Select a script..."))
            {
                for (const std::string &script_name : available_scripts)
                {
                    if (ImGui::Selectable(script_name.c_str()))
                    {
                        VortexMonoBehaviour *new_script = ScriptRegistry::get().create(script_name);
                        if (new_script)
                        {
                            model->add_behaviour(script_name, new_script);
                        }
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Spacing();
            ImGui::Checkbox("Show Collider", &model->show_collider);

            if (model->show_collider)
            {
                ImGui::Indent();
                
                ImGui::DragFloat3("Collider Scale", &model->collider_scale.x, 0.01f, 0.01f, 10.0f);
                
                float collider_uniform_scale = model->collider_scale.x;
                if (ImGui::SliderFloat("Uniform Scale##Collider", &collider_uniform_scale, 0.001f, 10.0f))
                {
                    model->collider_scale = glm::vec3(collider_uniform_scale);
                }

                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

                if (ImGui::Button("Reset Collider Size", ImVec2(-1, 0)))
                {
                    model->collider_scale = glm::vec3(1.0f);
                }

                ImGui::PopStyleColor(2);

                ImGui::Unindent();
            }

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
                ImGui::DragFloat3("##pos", &model->transform.position.x, 0.1f);

                ImGui::TableNextColumn();
                ImGui::Text("Scale");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat3("##scale", &model->transform.scale.x, 0.01f, 0.001f, 10.0f);

                ImGui::TableNextColumn();
                ImGui::Text("Rotation");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::DragFloat3("##rot", &model->transform.rotation.x, 0.1f);

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::SeparatorText("Scaling");

            float uniform_scale = model->transform.scale.x;
            if (ImGui::SliderFloat("Uniform Scale", &uniform_scale, 0.001f, 10.0f))
            {
                model->transform.scale = glm::vec3(uniform_scale);
            }

            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

            if (ImGui::Button("Reset Transform", ImVec2(-1, 0)))
            {
                model->transform.position = glm::vec3(0.0f);
                model->transform.scale = glm::vec3(1.0f);
                model->transform.rotation = glm::vec3(0.0f);
            }

            if (ImGui::Button("Delete Model", ImVec2(-1, 0)))
            {
                model->should_destroy = true;
            }

            ImGui::PopStyleColor(2);

            ImGui::PopStyleVar(3);
            ImGui::PopID();

            ImGui::Spacing();
        }
    }

    if (ps)
    {
        if (m_processed_ps.find(ps) != m_processed_ps.end()) return;
        m_processed_ps.insert(ps);

        std::string ps_header_id = "Particle System (" + ps->name +  ")##" + std::to_string((uintptr_t)ps);

        if (ImGui::CollapsingHeader(ps_header_id.c_str()))
        {
            ImGui::PushID(ps);
            
            ImGui::SeparatorText("Emitters");
            for (auto& [name, settings] : ps->emitter_registry)
            {
                ImGui::PushID(name.c_str());
                ImGui::Checkbox(name.c_str(), &settings.enabled);

                if (settings.enabled)
                {
                    ImGui::Indent();
                    ImGui::DragFloat3("Position", &settings.position.x, 0.1f);

                    VortexGuiLambda::ClampedInputInt("Spawn Rate", &settings.spawn_rate, 100, 500, 0, ps->max_particles);
                    VortexGuiLambda::ClampedInputFloat("Size", &settings.size, 0.1f, 1.0f, 0.05f, 5.0f);
                    VortexGuiLambda::ClampedInputFloat("Life", &settings.life, 0.5f, 1.0f, 0.1f, 10.0f);
                    VortexGuiLambda::ClampedInputFloat("Gravity", &settings.gravity, 0.2f, 0.5f, -2.0f, 2.0f);

                    ImGui::SliderFloat("Drag", &settings.drag, 0.0f, 5.0f);
                    ImGui::SliderFloat("Elasticity", &settings.elasticity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Friction", &settings.friction, 0.0f, 1.0f);

                    const char *behaviour_options[] = {"Grow", "Shrink", "None"};
                    int current_behaviour = static_cast<int>(settings.behaviour);

                    if (ImGui::Combo("Behaviour", &current_behaviour, behaviour_options, IM_ARRAYSIZE(behaviour_options)))
                    {
                        settings.behaviour = static_cast<ParticleBehaviour>(current_behaviour);
                    }

                    ImGui::ColorEdit4("Color", glm::value_ptr(settings.color));
                    ImGui::SeparatorText("Point Gravity");
                    ImGui::Checkbox("Use Point Gravity", &settings.use_point_gravity);

                    if (settings.use_point_gravity)
                    {
                        ImGui::DragFloat3("Target Pos", glm::value_ptr(settings.gravity_point), 0.1f);
                        VortexGuiLambda::ClampedInputFloat("Pull Strength", &settings.point_gravity_strength, 0.1f, 0.0f, 0.1f, 1.0f);
                    }

                    ImGui::Unindent();
                }

                ImGui::PopID();
                ImGui::Spacing();
            }

            ImGui::SeparatorText("Stats");
            ImGui::Text("Total Particles: %d", (int)ps->max_particles);
            ImGui::Text("Active Particles: %d", ps->active_count);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));

            if (ImGui::Button("Delete Particle System", ImVec2(-1, 0)))
            {
                ps->should_destroy = true;
            }

            ImGui::PopStyleColor(2);

            ImGui::PopID();
            ImGui::Spacing();
        }
    }
}

void VortexGUI::end_scene_inspector()
{
    if (!show_gui) return;

    ImGui::End();
}

void VortexGUI::creator_window(
    VortexApplication *window,
    std::vector<ParticleSystem*> &active_systems,
    std::vector<VortexModel*> &active_models
)
{
    if (!show_gui) return;

    float padding = 10.0f;
    ImVec2 next_pos = ImVec2(m_scene_pos.x, m_scene_pos.y + m_scene_size.y + padding);
    ImGui::SetNextWindowPos(next_pos, ImGuiCond_Always);

    float app_window_height = ImGui::GetIO().DisplaySize.y;
    float max_height = app_window_height - next_pos.y - 10.0f;
    if (max_height < 50.0f) max_height = 50.0f;
    ImGui::SetNextWindowSizeConstraints(ImVec2(350, 50), ImVec2(FLT_MAX, max_height));

    if (m_force_creator_collapse)
    {
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
        m_force_creator_collapse = false;
    }

    ImGui::Begin("Creator Tools", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::SeparatorText("File Management");
    
    static char project_name_buf[64] = "My_Project_Name";
    ImGui::InputText("Project Name", project_name_buf, IM_ARRAYSIZE(project_name_buf));

    if (ImGui::Button("Save Project", ImVec2(160, 30)))
    {
        VortexProject::save_project(std::string(project_name_buf), active_models, active_systems);
    }
    if (ImGui::Button("Load Project", ImVec2(160, 30)))
    {
        VortexProject::load_project(std::string(project_name_buf), active_models, active_systems, window);
    }

    ImGui::Spacing();

    bool is_collapsed = ImGui::IsWindowCollapsed();
    if (!is_collapsed && m_creator_was_collapsed)
    {
        m_force_scene_collapse = true;
    }
    m_creator_was_collapsed = is_collapsed;

    if (!is_collapsed)
    {
        ImGui::SeparatorText("New Script");

        static char new_script_name[64] = "MyNewBehaviour";
        ImGui::InputText("Class Name", new_script_name, IM_ARRAYSIZE(new_script_name));

        static bool show_script_error = false;
        static bool show_script_success = false;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.3f, 0.8f, 1.0f));

        if (ImGui::Button("Generate Script", ImVec2(-1, 0)))
        {
            std::string class_name = std::string(new_script_name);
            std::string file_path = "../Vortex/Game/Scripts/" + class_name + ".cpp";

            if (std::filesystem::exists(file_path))
            {
                show_script_error = true;
                show_script_success = false;
            }
            else if (!class_name.empty())
            {
                show_script_error = false;
                show_script_success = true;

                std::ofstream script_file(file_path);

                script_file << "#include \"vortex_behaviour.hpp\"\n";
                script_file << "#include \"util/vortex_script_registry.hpp\"\n";
                script_file << "#include \"vortex_keyboard.hpp\"\n";
                script_file << "#include \"vortex_mouse.hpp\"\n";
                script_file << "#include \"vortex_physics.hpp\"\n\n";
                
                script_file << "class " << class_name << " : public VortexMonoBehaviour\n";
                script_file << "{\n";
                script_file << "private:\n";
                script_file << "public:\n";
                script_file << "    void on_start() override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run once as initialization\n";
                script_file << "    }\n\n";
                script_file << "    void on_update(float deltaTime) override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run every frame\n";
                script_file << "    }\n\n";
                script_file << "    void late_update(float deltaTime) override\n";
                script_file << "    {\n";
                script_file << "        // write code here to run at the end of every frame\n";
                script_file << "    }\n";
                script_file << "};\n\n";
                
                script_file << "VORTEX_REGISTER_SCRIPT(" << class_name << ");\n";
                
                script_file.close();
                
                std::cout << "[EDITOR] Auto-Generated Script: " << file_path << std::endl;
            }
        }
        ImGui::PopStyleColor(2);

        if (show_script_error)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Error: Script already exists!");
        }
        if (show_script_success)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Generated! Restart engine to compile.");
        }
        ImGui::Spacing();

        ImGui::SeparatorText("New Particle System");

        static char new_ps_name[64] = "Magic_Dust";
        static int new_ps_max = 100000;

        ImGui::InputText("System Name", new_ps_name, IM_ARRAYSIZE(new_ps_name));
        VortexGuiLambda::ClampedInputInt("Max Capacity", &new_ps_max, 10000, 50000, 100, 1000000);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));

        static bool show_error_message_ps = false;

        if (ImGui::Button("Create Particle System", ImVec2(-1, 0)))
        {
            std::string target_name = std::string(new_ps_name);
            bool name_exists = false;

            for (ParticleSystem *ps : active_systems)
            {
                if (ps->name == target_name)
                {
                    name_exists = true;
                    break;
                }
            }

            if (name_exists)
            {
                show_error_message_ps = true;
            }
            else
            {
                show_error_message_ps = false;

                ParticleSystem *new_ps = new ParticleSystem(new_ps_max, window, std::string(new_ps_name));
                new_ps->get_emitter("Default Emitter");
                active_systems.push_back(new_ps);
            }
        }

        if (show_error_message_ps)
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Particles System with name(id) already exists!");
        }

        ImGui::PopStyleColor(2);

        ImGui::SeparatorText("New Model");

        if (ImGui::Button("Refresh Folder") || !m_models_scanned)
        {
            m_available_model_names.clear();
            m_available_model_paths.clear();

            std::string path = "assets/models/obj";

            if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
            {
                for (const auto &entry : std::filesystem::directory_iterator(path))
                {
                    if (entry.path().extension() == ".obj")
                    {
                        m_available_model_names.push_back(entry.path().stem().string());
                        m_available_model_paths.push_back(entry.path().string());
                    }
                }
            }
            m_models_scanned = true;
        }

        ImGui::Spacing();
        ImGui::BeginChild("ModelBrowser", ImVec2(0, 300), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        if (m_available_model_names.empty())
        {
            ImGui::TextDisabled("No .obj files found in:");
            ImGui::TextDisabled("assets/models/obj");
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));

            float button_double_height = ImGui::GetTextLineHeightWithSpacing() * 2.0f;
            for (size_t i = 0; i < m_available_model_names.size(); i++)
            {
                if (ImGui::Button(m_available_model_names[i].c_str(), ImVec2(-1, button_double_height)))
                {
                    VortexModel *new_model = new VortexModel(m_available_model_paths[i].c_str(), window);
                    active_models.push_back(new_model);
                }
            }
            ImGui::PopStyleColor(2);
        }
        ImGui::EndChild();
    }

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

void VortexGUI::post_process_options(VortexApplication *window)
{
    if (!show_gui) return;
    if (!m_shaders_loaded) refresh_shader_list();

    ImGui::SetNextWindowPos(ImVec2(180, 105), ImGuiCond_FirstUseEver);
    ImGui::Begin("Post-Processing", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (ImGui::Combo("Effect", &m_selected_shader_idx, VortexGuiLambda::DataGetter, static_cast<void*>(&m_display_names), static_cast<int>(m_display_names.size())))
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

void VortexGUI::skybox_options(VortexApplication *window)
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
            if (m_selected_skybox_idx == 0)
            {
                window->set_skybox(nullptr);
            }
            else
            {
                window->set_skybox(new VortexSkybox(m_skybox_files[m_selected_skybox_idx]));
            }
        }
    }

    if (ImGui::Button("Refresh Skyboxes")) m_skybox_loaded = false;

    ImGui::End();
}

VortexGUI::~VortexGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_processed_models.clear();
    m_processed_ps.clear();

    m_shader_files.clear();
    m_display_names.clear();

    for (std::vector<std::string> file : m_skybox_files)
    {
        file.clear();
    }
    m_skybox_files.clear();
    m_skybox_display_names.clear();

    m_available_model_names.clear();
    m_available_model_paths.clear();
}
