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

# Include FetchContent to download external libaries
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

FetchContent_MakeAvailable(glfw glm)
 
# Set the output directory
set(${PROJECT_NAME}_SOURCES
    "src/main.cpp"
)

# Add the project executable
add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)

## Add custom command to run compile.bat every time the project is built
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#       COMMAND ${CMAKE_COMMAND} -E echo "Running compile.bat..."
#       COMMAND ${CMAKE_COMMAND} -E env "PATH=$ENV{PATH}" compile.bat
#       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# )

# Copy resources to output folder
set(RESOURCES_SOURCE_DIR "./${CMAKE_CURRENT_SOURCE_DIR}/shaders")
file(GLOB_RECURSE RESOURCE_FILES
    "${SHADERS_SOURCE_DIR}/*.spv"
)
set(RESOURCES_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/")
file(MAKE_DIRECTORY ${RESOURCES_OUT_DIR})
foreach(RESOURCE ${RESOURCE_FILES})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE}
    ${RESOURCES_OUT_DIR})
endforeach(RESOURCE)