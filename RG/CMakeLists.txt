cmake_minimum_required(VERSION 3.12)
project(VehicleSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(assimp REQUIRED)

# Add ImGui source files
file(GLOB IMGUI_SOURCES
    vendor/imgui/*.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)

# Add your source files
add_executable(VehicleSimulator
    src/main.cpp
    src/OBJLoader/Model.cpp
    ${IMGUI_SOURCES}
)

# Link libraries
target_link_libraries(VehicleSimulator PRIVATE
    glfw
    OpenGL::GL
    assimp::assimp
)

# Include directories
target_include_directories(VehicleSimulator PRIVATE
    vendor/imgui
    vendor/glm
    vendor/glad/include
)

# Add GLAD
add_subdirectory(vendor/glad)
target_link_libraries(VehicleSimulator PRIVATE glad)

# Add after other commands
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})