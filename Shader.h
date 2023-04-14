// shader.h
#pragma once

#include <string>
#include <GL/glew.h>

class Shader {
public:
  Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
  ~Shader();

  void use() const;

private:
  GLuint program;

  GLuint loadShader(const std::string& shaderPath, GLenum shaderType);
  void checkShaderErrors(GLuint shader, const std::string& shaderPath, GLenum statusType);
};
