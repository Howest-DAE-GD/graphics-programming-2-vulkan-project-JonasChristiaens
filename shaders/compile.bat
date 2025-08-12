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
%GLSLC% "%SCRIPT_DIR%shader.vert" -o "%SCRIPT_DIR%vert.spv"
%GLSLC% "%SCRIPT_DIR%shader.frag" -o "%SCRIPT_DIR%frag.spv"
%GLSLC% "%SCRIPT_DIR%depthShader.frag" -o "%SCRIPT_DIR%depth_frag.spv"

pause
