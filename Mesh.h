// mesh.h
// Contains all the "header" variables, methods and structs
#pragma once

#include <vector>
#include <GL/glew.h>

struct Vertex {
  GLfloat position[3];
  GLfloat color[3];
};

class Mesh {
public:
  // Efficient constructor avoids overhead of copying vertices
  // By creating a single reference, a const
  Mesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
  ~Mesh(); // destructor

  void draw() const;

private:
  GLuint VAO, VBO, EBO;
  GLsizei indexCount;

  void setupMesh(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
};