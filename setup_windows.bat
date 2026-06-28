@echo off
setlocal enabledelayedexpansion
title Vortex Engine Setup

echo [Vortex Setup] Checking for required build tools...

:: Check if CMake is installed and in the system PATH
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [Vortex Setup] CMake not found! 
    echo Please install CMake or Visual Studio Build Tools with C++ workloads.
    echo You can install CMake quickly via winget: winget install -e --id Kitware.CMake
    pause
    exit /b 1
)

set "CURRENT_DIR=%CD%"

echo [Vortex Setup] Building Vortex Engine...

if not exist build\windows mkdir build\windows
cd build\windows

:: Generate project files (Defaults to Visual Studio if installed)
cmake -DCMAKE_BUILD_TYPE=Release ..\..
if %errorlevel% neq 0 (
    echo [Vortex Setup] CMake generation failed! Make sure you have a C++ compiler installed.
    pause
    exit /b 1
)

:: Build the engine using all available cores
cmake --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo [Vortex Setup] Build failed! Please check the terminal output for errors.
    pause
    exit /b 1
)

:: Depending on the generator (MSVC vs Ninja/MinGW), the executable might be in a Release subfolder
if exist "Release\engine.exe" (
    set "ENGINE_EXE=%CURRENT_DIR%\build\windows\Release\engine.exe"
) else if exist "engine.exe" (
    set "ENGINE_EXE=%CURRENT_DIR%\build\windows\engine.exe"
) else (
    echo [Vortex Setup] engine.exe not found! Build output path may have changed.
    pause
    exit /b 1
)

cd "%CURRENT_DIR%"

echo [Vortex Setup] Integrating Vortex Engine into the Start Menu...

:: Define the Start Menu Programs directory
set "START_MENU_DIR=%APPDATA%\Microsoft\Windows\Start Menu\Programs\Vortex Engine"
if not exist "%START_MENU_DIR%" mkdir "%START_MENU_DIR%"

set "ICON_PATH=%CURRENT_DIR%\Vortex\assets\images\branding\vortex_icon.png"
if not exist "%ICON_PATH%" (
    echo [Vortex Setup] Warning: vortex_icon.png not found. Please verify the path!
)

:: Use PowerShell to cleanly generate the Windows .lnk shortcut
set "PS_SCRIPT=$WshShell = New-Object -comObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('%START_MENU_DIR%\Vortex Engine.lnk'); $Shortcut.TargetPath = '%ENGINE_EXE%'; $Shortcut.WorkingDirectory = '%CURRENT_DIR%\build\windows'; $Shortcut.IconLocation = '%ICON_PATH%'; $Shortcut.Description = 'Custom C++ 3D Game Engine'; $Shortcut.Save()"

powershell -NoProfile -Command "%PS_SCRIPT%"

echo.
echo [Vortex Setup] Installation and build complete! 
echo You can now launch Vortex Engine directly from your Start Menu.
pause
