# Engine Manual Setup

1. **Install dependencies**

Troubleshooting: You can check for missing packages by generating the build files with cmake -B build. CMake will output a list of any missing dependencies (like Wayland, X11, or OpenGL headers) that you need to install via your package manager.

2. **Compile the engine**

Once your dependencies are installed, generate the build files and compile using all available CPU cores:

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(proc)
```

3. Run the engine

```bash
./engine
```

4. **Manual Desktop Integration (Optional)**

If you compiled manually but still want Vortex Engine to appear in your system's application launcher, you can create the app shortcut yourself.

First, copy the icon to your system's icon folder:

```bash
mkdir -p ~/.local/share/icons
cp ../Vortex/assets/images/branding/vortex_icon.png ~/.local/share/icons/vortex_icon.png
```

Next, create the .desktop file:

```bash
nano ~/.local/share/applications/vortex-engine.desktop
```

Paste the following configuration into the file. **Important**: Replace /path/to/Vortex-Game-Engine with the actual absolute path to your cloned repository:

```bash
[Desktop Entry]
Version=1.0
Type=Application
Name=Vortex Engine
Comment=Custom C++ 3D Game Engine
Exec=env -C "/path/to/Vortex-Game-Engine/Vortex" "/path/to/Vortex-Game-Engine/build/engine"
Icon=vortex_icon
Terminal=false
Categories=Development;3DGraphics;
```

Finally, make the shortcut executable and update the desktop database so your OS recognizes it:


```bash
chmod +x ~/.local/share/applications/vortex-engine.desktop
update-desktop-database ~/.local/share/applications
```
