// mesh.cpp
#include "Mesh.h"

Mesh::Mesh(GLenum drawMode) : drawMode(drawMode) {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

void Mesh::update(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices) {
  indexCount = static_cast<GLsizei>(indices.size());
  GLsizei vboSize = static_cast<GLsizei>(vertices.size() * sizeof(Vertex));
  GLsizei eboSize = static_cast<GLsizei>(indices.size() * sizeof(GLuint));

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  if (vboSize > vboCapacity) {
    vboCapacity = vboSize * 2;
    glBufferData(GL_ARRAY_BUFFER, vboCapacity, nullptr, GL_DYNAMIC_DRAW);
  }
  glBufferSubData(GL_ARRAY_BUFFER, 0, vboSize, vertices.data());

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  if (eboSize > eboCapacity) {
    eboCapacity = eboSize * 2;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboCapacity, nullptr, GL_DYNAMIC_DRAW);
  }
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, eboSize, indices.data());

  glBindVertexArray(0);
}

void Mesh::draw() const {
  if (indexCount == 0) return;
  glBindVertexArray(VAO);
  glDrawElements(drawMode, indexCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
