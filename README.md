# Vortex Engine

Vortex Engine is a personal project currently in its early development stages. It serves as a foundational bridge between basic rendering and a complete game engine, focusing heavily on low-level systems like memory management and pipeline optimization.

While core features such as high-performance particle pooling and instanced rendering are functional, the engine is actively evolving. Many standard features, including complex lighting and a scene hierarchy, are on the roadmap. This project follows a bottom-up approach to ensure a fast, efficient, and highly optimized base for future implementations.

(Note: I am building this from scratch to learn engine architecture, so expect rough edges as I figure out what I am doing!)

## Prerequisites
Vortex Engine is currently built for Linux (Ubuntu/Debian). You will need git and cmake installed on your system.

## Installation and Building

1. Clone the repository

```bash
  git clone git@github.com:Mashhood-Husnain/Vortex-Game-Engine.git
  cd Vortex-Game-Engine
```

2. Install dependencies

The easiest way to get started is by running the included setup script, which will automatically fetch the required Linux headers and graphics libraries:

```bash
  ./setup_linux.sh
  ```

Troubleshooting: If the script fails, you can manually check for missing packages by running cmake -B build. CMake will output a list of any missing dependencies you need to install via your package manager.

3. Compile the engine

Once your dependencies are installed, generate the build files and compile:

```bash
  cmake -B build
  cd build
  make -j8
```

4. Run the engine

```bash
  ./engine
```

## Using the engine
### Scripting
All gameplay scripts are stored in the Vortex/Game/Scripts directory.
  - You can create new scripts directly through the engine's Creator Tool window.
  - Simply specify the name of the file, click create, and then open the file in your preferred IDE to write your C++ code.

Future Roadmap: Currently, you must close and recompile the engine after writing a new script. C++ Hot-reloading is planned for a future update so you won't have to restart the editor to see your code changes!

### Editor Controls
When you launch the engine, you will start in Editor Mode attached to the default camera. When you enter Play Mode, these default controls are disassociated from the camera so you can use them in your own game scripts.

| Key | Action (Editor Mode) |
| --- | --- |
| **W, A, S, D** | Move camera horizontally |
| **Q, E** | Move camera up / down |
| **M** | Release/Capture Mouse (Use this to interact with the Creator Tool or Inspector) |
| **T** | Toggle Wireframe view for all models |
| **F** | Toggle Fullscreen / Windowed mode |
| **Escape** | Exit Play Mode (if playing) OR Exit Engine (if in editor) |
