#include "vortex_gui.hpp"
#include <imgui_internal.h>
#include "vortex_application.hpp"

void VortexGUI::build_dockspace()
{
    ImGuiID dockspace_id = ImGui::GetID("VortexDockSpace");

    static bool first_time = true;
    if (first_time)
    {
        first_time = false;

        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            ImGuiID dock_main_id = dockspace_id;

            ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);

            ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

            ImGuiID dock_left_bottom = ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Down, 0.20f, nullptr, &dock_left);
            ImGuiID dock_left_top = dock_left;

            ImGuiID dock_middle_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.30f, nullptr, &dock_main_id);

            // Main View
            ImGui::DockBuilderDockWindow("Scene View", dock_main_id);

            // Render View
            ImGui::DockBuilderDockWindow("Render View", dock_main_id);

            // Text view
            ImGui::DockBuilderDockWindow("File Viewer", dock_main_id);

            // Image View
            ImGui::DockBuilderDockWindow("Image Viewer", dock_main_id);

            // Left Region
            ImGui::DockBuilderDockWindow("Scene Inspector", dock_left_top);
            ImGui::DockBuilderDockWindow("SkyBox", dock_left_bottom);
            ImGui::DockBuilderDockWindow("Post-Processing", dock_left_bottom);

            // Right Region
            ImGui::DockBuilderDockWindow("Creator Tools", dock_right);

            // Middle-Bottom Region
            ImGui::DockBuilderDockWindow("Asset Browser", dock_middle_bottom);
            // ImGui::DockBuilderDockWindow("Performance", dock_middle_bottom);
            // ImGui::DockBuilderDockWindow("Camera", dock_middle_bottom);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    ImGui::DockSpaceOverViewport(dockspace_id, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void VortexGUI::resize_scene_fbo(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    scene_width = static_cast<float>(width);
    scene_height = static_cast<float>(height);

    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);

    glBindTexture(GL_TEXTURE_2D, scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, scene_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VortexGUI::resize_render_scene_fbo(int width, int height)
{
    if (width <= 0 || height <= 0) return;

    render_scene_width = static_cast<float>(width);
    render_scene_height = static_cast<float>(height);

    glBindFramebuffer(GL_FRAMEBUFFER, render_scene_fbo);
    glBindTexture(GL_TEXTURE_2D, render_scene_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, render_scene_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VortexGUI::setup_scene_fbo(int width, int height)
{
    scene_width = static_cast<float>(width);
    scene_height = static_cast<float>(height);

    glGenFramebuffers(1, &scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);

    glGenTextures(1, &scene_texture);
    glBindTexture(GL_TEXTURE_2D, scene_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_texture, 0);

    glGenRenderbuffers(1, &scene_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, scene_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, scene_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        VORTEX_ERROR("ERROR::FRAMEBUFFER:: Scene Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VortexGUI::destroy_scene()
{
    if (scene_texture != 0)
    {
        glDeleteTextures(1, &scene_texture);
        scene_texture = 0;
    }

    if (scene_rbo != 0)
    {
        glDeleteRenderbuffers(1, &scene_rbo);
        scene_rbo = 0;
    }

    if (scene_fbo != 0)
    {
        glDeleteFramebuffers(1, &scene_fbo);
        scene_fbo = 0;
    }
}

void VortexGUI::setup_render_scene_fbo(int width, int height)
{
    render_scene_width = static_cast<float>(width);
    render_scene_height = static_cast<float>(height);

    glGenFramebuffers(1, &render_scene_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_scene_fbo);

    glGenTextures(1, &render_scene_texture);
    glBindTexture(GL_TEXTURE_2D, render_scene_texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_scene_texture, 0);

    glGenRenderbuffers(1, &render_scene_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_scene_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_scene_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        VORTEX_ERROR("ERROR::FRAMEBUFFER:: Render Scene Framebuffer is not complete!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VortexGUI::destroy_render_scene()
{
    if (render_scene_texture != 0)
    {
        glDeleteTextures(1, &render_scene_texture);
        render_scene_texture = 0;
    }

    if (render_scene_rbo != 0)
    {
        glDeleteRenderbuffers(1, &render_scene_rbo);
        render_scene_rbo = 0;
    }

    if (render_scene_fbo != 0)
    {
        glDeleteFramebuffers(1, &render_scene_fbo);
        render_scene_fbo = 0;
    }
}

void VortexGUI::bind_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
}

void VortexGUI::bind_render_framebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, render_scene_fbo);
}

