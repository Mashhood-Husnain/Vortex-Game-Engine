#pragma once
#include "vortex_canvas.hpp"
#include <vector>
#include <string>

class VortexUIManager
{
public:
    static std::vector<VortexCanvas*> active_canvases;

    static void add_canvas(VortexCanvas* canvas);
    static VortexCanvas* get_canvas(const std::string& name);
    
    static void render_ui();
    static void cleanup();
};
