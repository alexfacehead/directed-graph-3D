# directed-graph-3D: A 3D Directed Graph Renderer

This is a simple C++ project that demonstrates how to render a 3D directed graph using OpenGL, GLFW, GLEW, and glm.

## Prerequisites

The following dependencies are required:

- GLFW 3.x.x
- GLEW 2.x.x
- glm 0.9.x

## Installation

### Debian-based systems (e.g., Ubuntu)

Install the required dependencies using the following command:

`sudo apt-get install libglfw3-dev libglew-dev libglm-dev`


### Other platforms

Please refer to the respective package manager or build system for installing the required libraries.

## Building

1. Navigate to the project directory.
2. Compile the project using your preferred C++ compiler, for example:

`g++ -o main main.cpp Mesh.cpp Material.cpp Renderer.cpp -lGLEW -lglfw -lGL -ldl`


## Running

After building the project, run the compiled executable:

`./main`


You should see a window displaying the 3D directed graph.

This README was optimized by GPT-4.