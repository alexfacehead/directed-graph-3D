// shader.cpp
#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
  GLuint vertexShader = loadShader(vertexShaderPath, GL_VERTEX_SHADER);
  GLuint fragmentShader = loadShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

  program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  checkShaderErrors(program, "PROGRAM", GL_LINK_STATUS);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() {
  glDeleteProgram(program);
}

void Shader::use() const {
  glUseProgram(program);
}

GLuint Shader::loadShader(const std::string& shaderPath, GLenum shaderType) {
  std::ifstream file(shaderPath);
  if (!file) {
    throw std::runtime_error("Failed to open shader file: " + shaderPath);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string shaderSource = buffer.str();

  const GLchar* source = shaderSource.c_str();
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  checkShaderErrors(shader, shaderPath, GL_COMPILE_STATUS);

  return shader;
}

void Shader::checkShaderErrors(GLuint shader, const std::string& shaderPath, GLenum statusType) {
    GLint success;
    if (statusType == GL_COMPILE_STATUS) {
        glGetShaderiv(shader, statusType, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION::" << shaderPath << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, statusType, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::LINKING::" << shaderPath << "\n" << infoLog << std::endl;
        }
    }
}