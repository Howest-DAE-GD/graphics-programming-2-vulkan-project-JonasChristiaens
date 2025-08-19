# Vulkan Deferred Renderer

A modern real-time renderer built with Vulkan, featuring a fully deferred shading pipeline, physically-based rendering (PBR), and dynamic post-processing for the Graphics Programming 2 course at Howest DAE. Designed for extensibility, realism, and educational clarity, the engine showcases advanced graphics techniques without relying on legacy Vulkan render passes.

## Features

- **Deferred Rendering Architecture**
  - Scene geometry is rendered into a G-buffer (multiple render targets) storing position, normal, albedo, and material properties.
  - Lighting calculations are decoupled from geometry rendering, enabling efficient support for multiple dynamic lights.

- **Physically-Based Rendering (PBR)**
  - Implements GGX microfacet distribution, Smith geometry term, and Fresnel-Schlick approximation for realistic lighting and materials.
  - Supports real-world material properties: roughness, metallic, and albedo textures.

- **Alpha Clipping**
  - Transparent surfaces are handled correctly in depth and geometry passes, improving visual fidelity for masked objects.

- **Dynamic Tone Mapping and Exposure**
  - High Dynamic Range (HDR) rendering and filmic tone mapping (Uncharted2 curve).
  - Real-time camera controls for aperture, shutter speed, and ISO, simulating photographic exposure.

- **Pipeline Stages**
  - **Depth Prepass:** Generates a depth buffer for early visibility culling and transparency handling.
  - **Geometry Pass:** Populates the G-buffer with per-pixel scene data.
  - **Lighting Pass:** Computes per-pixel PBR lighting based on G-buffer and depth information.
  - **Tone Mapping Pass:** Converts HDR image to LDR for display, applying exposure and filmic tone mapping.

## Limitations

- **No Shadow Mapping:** Objects do not cast physical shadows from lights. All visible pixels receive illumination from all light sources.

## Getting Started

The project is organized with clear separation between device management, swapchain, descriptor sets, pipeline creation, command recording, and scene management. Shaders for each pipeline stage are customizable and located in the `shaders/` directory.

### Build & Run

1. Install Vulkan SDK.
2. Clone the repository and initialize submodules if needed.
3. Build using your preferred C++ toolchain.
4. Run the executable to launch the real-time demo.

### Directory Structure

- `src/` – Core engine source code
- `shaders/` – GLSL shaders for each pipeline stage
- `models/` – Models and textures
- `README.md` – Project overview

## Special Techniques

- **Deferred G-buffer:** Efficient multi-target rendering for lighting flexibility.
- **Advanced PBR:** Realistic materials and lighting.
- **Transparency Handling:** Masked textures behave correctly in depth and color passes.
- **Dynamic Exposure:** Camera parameters affect image brightness in real-time.

Name: Jonas Christiaens 2DAE11
Github link: https://github.com/Howest-DAE-GD/graphics-programming-2-vulkan-project-JonasChristiaens