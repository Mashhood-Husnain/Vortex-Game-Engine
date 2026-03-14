#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <set>

class VortexModel;

class VortexGUI
{
    std::set<VortexModel*> m_processed_models;
public:
    VortexGUI();
    ~VortexGUI();

    void init(GLFWwindow* window);
    void update();
    void render();
    void begin_scene_inspector();
    void end_scene_inspector();
    void show_engine_stats();
    void show_model_info(VortexModel *model);
};
