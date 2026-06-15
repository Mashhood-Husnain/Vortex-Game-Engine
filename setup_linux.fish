#!/usr/bin/env fish

echo "[Vortex Setup] Detecting Operating System..."

if test -f /etc/os-release
    set OS_ID (grep -E '^ID=' /etc/os-release | cut -d '=' -f 2 | tr -d '"')
    set OS_ID_LIKE (grep -E '^ID_LIKE=' /etc/os-release | cut -d '=' -f 2 | tr -d '"')

    if test "$OS_ID" = "ubuntu"; or test "$OS_ID" = "debian"; or string match -q '*ubuntu*' "$OS_ID_LIKE"
        echo "[Vortex Setup] Ubuntu/Debian detected. Installing dependencies..."
        sudo apt update
        sudo apt install -y build-essential git cmake pkg-config libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev wayland-protocols libgl1-mesa-dev
        echo "[Vortex Setup] Dependencies installed successfully!"

    else if test "$OS_ID" = "arch"; or string match -q '*arch*' "$OS_ID_LIKE"
        echo "[Vortex Setup] Arch Linux detected. Installing dependencies..."
        sudo pacman -Syu --noconfirm base-devel git cmake pkgconf libx11 libxrandr libxinerama libxcursor libxi libxkbcommon wayland wayland-protocols mesa
        echo "[Vortex Setup] Dependencies installed successfully!"

    else
        echo "[Vortex Setup] Unsupported Linux distribution: $OS_ID"
        echo "Please install X11, Wayland, and OpenGL development headers manually."
    end
else
    echo "[Vortex Setup] Cannot determine OS. Are you on Linux?"
end

set CURRENT_DIR (pwd)

echo "[Vortex Setup] Building Vortex Engine..."

mkdir -p build
cd build; or exit

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j(nproc)

if not test -f "engine"
    echo "[Vortex Setup] Build failed! Please check the terminal output for errors."
    exit 1
end

cd "$CURRENT_DIR"; or exit

echo "[Vortex Setup] Integrating Vortex Engine into application launcher..."

set APPS_DIR "$HOME/.local/share/applications"
set ICONS_DIR "$HOME/.local/share/icons"

mkdir -p "$APPS_DIR"
mkdir -p "$ICONS_DIR"

if test -f "$CURRENT_DIR/Vortex/assets/images/branding/vortex_icon.png"
    cp "$CURRENT_DIR/Vortex/assets/images/branding/vortex_icon.png" "$ICONS_DIR/"
else
    echo "[Vortex Setup] Warning: vortex_icon.png not found. Please verify the path!"
end

printf "[Desktop Entry]\n\
Version=1.0\n\
Type=Application\n\
Name=Vortex Engine\n\
Comment=Custom C++ 3D Game Engine\n\
Exec=env -C \"%s/Vortex\" \"%s/build/engine\"\n\
Icon=vortex_icon\n\
Terminal=false\n\
Categories=Development;3DGraphics;\n" "$CURRENT_DIR" "$CURRENT_DIR" > "$APPS_DIR/vortex-engine.desktop"

chmod +x "$CURRENT_DIR/build/engine"
chmod +x "$APPS_DIR/vortex-engine.desktop"

update-desktop-database "$APPS_DIR"

echo "[Vortex Setup] Installation and build complete! You can now launch Vortex Engine from your app menu."
