// shader.h
#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader {
public:
  Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
  ~Shader();

  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  void use() const;
  GLuint getProgram() const;
  void setMat4(const std::string& name, const glm::mat4& value) const;

private:
  GLuint program = 0;
  mutable std::unordered_map<std::string, GLint> uniformCache;

  GLint getUniformLocation(const std::string& name) const;
  GLuint loadShader(const std::string& shaderPath, GLenum shaderType);
  void checkShaderErrors(GLuint shader, const std::string& shaderPath, GLenum statusType);
};
