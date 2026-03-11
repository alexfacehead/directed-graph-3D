cmake_minimum_required(VERSION 3.16)
project(directed-graph-3D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress macOS OpenGL deprecation warnings
add_definitions(-DGL_SILENCE_DEPRECATION)

# Find dependencies
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(OpenGL REQUIRED)

# Fetch Dear ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(directed_graph
    src/Main.cpp
    src/Mesh.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Material.cpp
    src/Renderer.cpp
    src/Hypergraph.cpp
    src/ForceLayout.cpp
    src/RuleEngine.cpp
    src/GraphIO.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(directed_graph PRIVATE
    src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(directed_graph
    glfw
    GLEW::GLEW
    OpenGL::GL
    glm::glm
)

# Aggressive optimization: -O3, native arch, fast math, LTO
target_compile_options(directed_graph PRIVATE
    -O3
    -march=native
    -ffast-math
    -funroll-loops
)
set_property(TARGET directed_graph PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

# Copy shader files to build directory
configure_file(shaders/vertex_shader.glsl ${CMAKE_BINARY_DIR}/vertex_shader.glsl COPYONLY)
configure_file(shaders/fragment_shader.glsl ${CMAKE_BINARY_DIR}/fragment_shader.glsl COPYONLY)
