// mesh.h
// Contains all the "header" variables, methods and structs
#pragma once

#include <vector>
#include <glad/gl.h>

struct Vertex {
  GLfloat position[3];
  GLfloat color[3];
};

class Mesh {
public:
  Mesh(GLenum drawMode = GL_TRIANGLES);
  ~Mesh();

  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;

  void update(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices);
  void draw() const;

private:
  GLuint VAO, VBO, EBO;
  GLsizei indexCount = 0;
  GLenum drawMode;
  GLsizei vboCapacity = 0;
  GLsizei eboCapacity = 0;
};