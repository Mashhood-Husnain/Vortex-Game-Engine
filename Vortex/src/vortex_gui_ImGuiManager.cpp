#include "vortex_gui_ImGui.hpp"

void VortexImGuiManager::init(VortexApplication *app)
{
    context.application = app;

    std::shared_ptr<VortexTerminalWindow> terminal = std::make_shared<VortexTerminalWindow>();
    VortexImGuiManager::register_window(terminal);

    std::shared_ptr<VortexInspectorWindow> inspector = std::make_shared<VortexInspectorWindow>();
    VortexImGuiManager::register_window(inspector);
 
    std::shared_ptr<VortexCreatorWindow> creator = std::make_shared<VortexCreatorWindow>();
    VortexImGuiManager::register_window(creator);
}

void VortexImGuiManager::register_window(std::shared_ptr<VortexImGuiWindow> window)
{
    windows[window->window_name] = window;
}

void VortexImGuiManager::render_all()
{
    for (auto &pair : windows)
    {
        pair.second->render();
    }
}

void VortexImGuiManager::toggle_window(const std::string &name)
{
    if (windows.find(name) != windows.end())
    {
        windows[name]->is_open = !windows[name]->is_open;
    }
}

void VortexImGuiManager::set_visibility(const std::string &name, const bool is_visible)
{
    if (windows.find(name) != windows.end())
    {
        windows[name]->is_open = is_visible;
    }
}

bool VortexImGuiManager::get_visibility(const std::string &name)
{
    if (windows.find(name) != windows.end())
    {
        return windows[name]->is_open;
    }

    return false;
}
