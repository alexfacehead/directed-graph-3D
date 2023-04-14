#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    // Initialization failed
    return -1;
  }

  // Create a window with GLFW
  GLFWwindow *window = glfwCreateWindow(1024, 768, "My Window", NULL, NULL);
  if (!window) {
    // Window creation failed
    glfwTerminate();
    return -1;
  }

  // Make the window's OpenGL context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewInit();

  // Load shader files
  Shader shader("./vertex_shader.glsl", "./fragment_shader.glsl");

  // Material constructor requires these attributes
  glm::vec3 ambient(0.1f, 0.1f, 0.1f);
  glm::vec3 diffuse(0.5f, 0.5f, 0.5f);
  glm::vec3 specular(1.0f, 1.0f, 1.0f);
  float shininess = 32.0f;

  // Create a mesh, material, and renderer
  // You'll need to provide your own vertex and index data for the Mesh constructor
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;
  Mesh mesh(vertices, indices);

  Material material(shader, ambient, diffuse, specular, shininess);
  Renderer renderer;

  Camera camera(640.0f / 480.0f);

  std::vector<Vertex> nodeVertices = {
      // Node 1 vertices
      {{-0.1f, -0.1f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{0.1f, -0.1f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.1f, 0.1f, 0.0f}, {0.0f, 0.0f, 1.0f}},
      {{-0.1f, 0.1f, 0.0f}, {1.0f, 1.0f, 0.0f}},
  };


  std::vector<GLuint> nodeIndices = {
      0, 1, 2, // Node 1
      0, 2, 3, // Node 1
  };

  Mesh nodeMesh(nodeVertices, nodeIndices);

  std::vector<Vertex> lineVertices = {
      // Edge vertices
      {{0.1f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
      {{0.9f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
  };

  std::vector<GLuint> lineIndices = {
      0, 1, // Edge
  };

  Mesh edgeMesh(lineVertices, lineIndices);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
      // Clear the screen
      glClear(GL_COLOR_BUFFER_BIT);

      // Render the scene
      renderer.render(nodeMesh, material, camera);
      renderer.render(edgeMesh, material, camera);

      // Swap front and back buffers
      glfwSwapBuffers(window);

      // Poll for and process events
      glfwPollEvents();
    }
}