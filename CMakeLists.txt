# Set minimum version of CMake
cmake_minimum_required(VERSION 3.27)

# Project Name
project(GP2_Vulkan)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if Vulkan is installed
find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

# Include FetchContent to download external libraries
include(FetchContent)

# Fetch Vulkan GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  GIT_SHALLOW ON
)

# Fetch Vulkan GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  GIT_SHALLOW ON
)

# Fetch Vulkan VMA
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.2.1
  GIT_SHALLOW ON
)

# Fetch stb
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
  GIT_SHALLOW ON
)

# Fetch tinyobjloader
FetchContent_Declare(
  tob
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG v2.0.0-rc1
  GIT_SHALLOW ON
)

FetchContent_MakeAvailable(glfw glm stb tob)

# Set the output directory
set(${PROJECT_NAME}_SOURCES
    "src/main.cpp"
    "src/utils.cpp"
    "src/Window.cpp"
    "src/Instance.cpp"
    "src/Surface.cpp"
    "src/Device.cpp"
    "src/SwapChain.cpp"
    "src/Image.cpp"
    "src/RenderPass.cpp"
    "src/DescriptorSetLayout.cpp"
    "src/Pipeline.cpp"
)

# Add the project executable
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)

# Include the stb_image.h header
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR} ${tinyobjloader_SOURCE_DIR})

# Add custom command to run compile.bat every time the project is built
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running compile.bat..."
    COMMAND ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}" ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.bat
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# SHADERS FOLDER FUNCTIONALITY
# Set the shaders source and output directories
set(SHADERS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

# Ensure the output directory exists
file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})

# Copy all files after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SHADERS_SOURCE_DIR} ${SHADERS_OUT_DIR}
    COMMENT "Copying compiled shaders to build directory..."
)

# TEXTURES FOLDER FUNCTIONALITY
# Set the textures source and output directories
set(TEXTURES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/textures")
set(TEXTURES_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/textures/")

# Ensure the textures output directory exists
file(MAKE_DIRECTORY ${TEXTURES_OUT_DIR})

# Copy all texture files after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${TEXTURES_SOURCE_DIR} ${TEXTURES_OUT_DIR}
    COMMENT "Copying textures to build directory..."
)

# MODELS FOLDER FUNCTIONALITY
# Set the textures source and output directories
set(MODELS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/models")
set(MODELS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/models/")

# Ensure the models output directory exists
file(MAKE_DIRECTORY ${MODELS_OUT_DIR})

# Copy all models files after build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${MODELS_SOURCE_DIR} ${MODELS_OUT_DIR}
    COMMENT "Copying textures to build directory..."
)