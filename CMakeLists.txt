cmake_minimum_required(VERSION 3.16)
project(directed-graph-3D LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Suppress macOS OpenGL deprecation warnings
if(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

# --- All dependencies via FetchContent (no system installs needed) ---
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Dear ImGui
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

# GLAD (local, generated for OpenGL 3.3 Core)
set(GLAD_SOURCES glad/src/gl.c)

# OpenGL
find_package(OpenGL REQUIRED)

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
    ${GLAD_SOURCES}
)

target_include_directories(directed_graph PRIVATE
    src
    glad/include
    stb
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(directed_graph
    glfw
    OpenGL::GL
    glm::glm
)

# Platform-specific link libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(directed_graph dl)
endif()

# Optimization flags (platform-aware)
if(MSVC)
    target_compile_options(directed_graph PRIVATE /O2 /fp:fast)
else()
    target_compile_options(directed_graph PRIVATE
        -O3
        -ffast-math
        -funroll-loops
    )
    # Only use -march=native for local builds, not CI cross-platform
    if(NOT DEFINED ENV{CI})
        target_compile_options(directed_graph PRIVATE -march=native)
    endif()
endif()

if(NOT MSVC)
    set_property(TARGET directed_graph PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
