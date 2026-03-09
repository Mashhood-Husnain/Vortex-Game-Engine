# Vortex Engine

A lightweight, high-performance 2D/3D rendering engine built from scratch using **C++** and **OpenGL**. This project focuses on implementing efficient instanced rendering, modular shader systems, and custom particle physics.

## Prerequisites
Ensure you have a C++17 compatible compiler and CMake (3.10+) installed.
* **Linux (Ubuntu/Debian):** `sudo apt-get install build-essential cmake libx11-dev libglu1-mesa-dev freeglut3-dev libmesa-dev`

## Build Instructions

Follow these commands in your terminal to compile and launch the engine:

```bash
# 1. Generate the build directory and configuration
cmake -B build

# 2. Enter the build directory
cd build

# 3. Finalize CMake configuration
cmake ..

# 4. Compile the project
make

# 5. Run the program
./engine
