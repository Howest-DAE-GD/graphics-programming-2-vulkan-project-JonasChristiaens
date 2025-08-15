@echo off
setlocal

:: Check if VULKAN_SDK environment variable is set
if "%VULKAN_SDK%"=="" (
    echo [ERROR] VULKAN_SDK environment variable is not set!
    echo Please set it to the Vulkan SDK installation path.
    pause
    exit /b 1
)

:: Define GLSLC path
set GLSLC="%VULKAN_SDK%\Bin\glslc.exe"

:: Get the path to the script's directory
set SCRIPT_DIR=%~dp0

:: Compile shaders
%GLSLC% "%SCRIPT_DIR%depthShader.vert" -o "%SCRIPT_DIR%depth_vert.spv"
%GLSLC% "%SCRIPT_DIR%depthShader.frag" -o "%SCRIPT_DIR%depth_frag.spv"
%GLSLC% "%SCRIPT_DIR%geometry.vert" -o "%SCRIPT_DIR%geometry_vert.spv"
%GLSLC% "%SCRIPT_DIR%geometry.frag" -o "%SCRIPT_DIR%geometry_frag.spv"
%GLSLC% "%SCRIPT_DIR%lighting.frag" -o "%SCRIPT_DIR%lighting_frag.spv"
%GLSLC% "%SCRIPT_DIR%fullscreen.vert" -o "%SCRIPT_DIR%fullscreen_vert.spv"

pause
