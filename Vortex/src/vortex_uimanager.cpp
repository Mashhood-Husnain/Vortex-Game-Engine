#include "vortex_uimanager.hpp"

std::vector<VortexCanvas*> VortexUIManager::active_canvases;

void VortexUIManager::add_canvas(VortexCanvas* canvas)
{
    active_canvases.push_back(canvas);
}

VortexCanvas* VortexUIManager::get_canvas(const std::string& name)
{
    for (VortexCanvas* c : active_canvases)
    {
        if (c->canvas_name == name) return c;
    }
    return nullptr;
}

void VortexUIManager::render_ui()
{
    for (VortexCanvas* canvas : active_canvases)
    {
        canvas->draw();
    }
}

void VortexUIManager::cleanup()
{
    for (VortexCanvas* canvas : active_canvases)
    {
        delete canvas;
    }
    active_canvases.clear();
}
