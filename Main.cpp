#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    // Initialization failed
    return -1;
  }

  // Create a window with GLFW
  GLFWwindow *window = glfwCreateWindow(640, 480, "My Window", NULL, NULL);
  if (!window) {
    // Window creation failed
    glfwTerminate();
    return -1;
  }

  // Make the window's OpenGL context current
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewInit();

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Update your scene and perform any required calculations

    // Render your scene here

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Clean up resources and terminate GLFW
  glfwTerminate();
  return 0;
}
