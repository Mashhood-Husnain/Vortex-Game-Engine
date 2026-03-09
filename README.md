# Vortex Engine

Vortex Engine is a personal project currently in its early development stages. It serves as a foundational bridge between basic rendering and a complete engine, focusing on low-level systems like memory management and pipeline optimization. While core features such as high-performance particle pooling and instanced rendering are functional, the engine is not yet a finished toolkit. Many standard features, including complex lighting and a scene hierarchy, are still being planned. This project follows a bottom-up approach to ensure a fast and efficient base for future implementations.

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
