/*
 * File: world.cpp
 * Project: VortexEngine
 * Description: Running the engine
 * Author: Mashhood Husnain
 * License: MIT
 */

#include "vortex_application.hpp"

int main() {
    VortexApplication VortexEngineWindow("Vortex Engine", 900, 600);

    VortexEngineWindow.run([&](){});

    return 0;
}
