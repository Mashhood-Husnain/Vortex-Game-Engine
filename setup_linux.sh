#!/bin/bash

echo "[Vortex Setup] Detecting Operating System..."

if [ -f /etc/os-release ]; then
    . /etc/os-release
    
    if [[ "$ID" == "ubuntu" || "$ID" == "debian" || "$ID_LIKE" == *"ubuntu"* ]]; then
        echo "[Vortex Setup] Ubuntu/Debian detected. Installing dependencies..."
        sudo apt update
        sudo apt install -y build-essential git cmake pkg-config libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev wayland-protocols libgl1-mesa-dev
        echo "[Vortex Setup] Dependencies installed successfully!"
        
    elif [[ "$ID" == "arch" || "$ID_LIKE" == *"arch"* ]]; then
        echo "[Vortex Setup] Arch Linux detected. Installing dependencies..."
        sudo pacman -Syu --noconfirm base-devel git cmake pkgconf libx11 libxrandr libxinerama libxcursor libxi libxkbcommon wayland wayland-protocols mesa
        echo "[Vortex Setup] Dependencies installed successfully!"
        
    else
        echo "[Vortex Setup] Unsupported Linux distribution: $ID"
        echo "Please install X11, Wayland, and OpenGL development headers manually."
    fi
else
    echo "[Vortex Setup] Cannot determine OS. Are you on Linux?"
fi
