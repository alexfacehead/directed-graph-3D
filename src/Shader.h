// shader.h
#pragma once

#include <string>
#include <unordered_map>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Shader {
public:
  // Create from embedded source strings
  static Shader fromSource(const char* vertexSource, const char* fragmentSource);

  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  void use() const;
  GLuint getProgram() const;
  void setMat4(const std::string& name, const glm::mat4& value) const;

private:
  Shader() = default;
  GLuint program = 0;
  mutable std::unordered_map<std::string, GLint> uniformCache;

  GLint getUniformLocation(const std::string& name) const;
  static GLuint compileShader(const char* source, GLenum shaderType, const char* label);
  static void checkShaderErrors(GLuint shader, const char* label, GLenum statusType);
};
