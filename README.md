# Vortex Engine

Vortex Engine is a personal project currently in its early development stages. It serves as a foundational bridge between basic rendering and a complete engine, focusing on low-level systems like memory management and pipeline optimization. While core features such as high-performance particle pooling and instanced rendering are functional, the engine is not yet a finished toolkit. Many standard features, including complex lighting and a scene hierarchy, are still being planned. This project follows a bottom-up approach to ensure a fast and efficient base for future implementations. (I HAVE NO IDEA WHAT I'M DOING!!!)

## Prerequisites

Ensure you have a **C++17** compatible compiler and **CMake (3.10+)** installed.

* **Windows:** Install VScode
* **Linux (Ubuntu/Debian):**
```bash
  sudo apt-get update
  sudo apt-get install build-essential cmake libx11-dev libglu1-mesa-dev freeglut3-dev libmesa-dev libnvidia-ml-dev
  ```

## Build Instructions

This project uses CMake. Run the following commands from the root directory of the repository:

```bash
# 1. Generate the build files and configuration
cmake -S . -B build

# 2. Compile the project (Release configuration is recommended for performance)
cmake --build build --config Release
```

## Running the Engine

Once the compilation is complete, you can launch the engine using the command specific to your operating system:

* **Windows (PowerShell / Command Prompt):**
  ```cmd
  .\build\Release\engine.exe
  ```

* **Linux (Terminal):**
  ```bash
  cd build

  ./engine
  ```
