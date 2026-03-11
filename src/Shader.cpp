// shader.cpp
#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

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
  if (program) glDeleteProgram(program);
}

Shader::Shader(Shader&& other) noexcept : program(other.program), uniformCache(std::move(other.uniformCache)) {
  other.program = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
  if (this != &other) {
    if (program) glDeleteProgram(program);
    program = other.program;
    uniformCache = std::move(other.uniformCache);
    other.program = 0;
  }
  return *this;
}

void Shader::use() const {
  glUseProgram(program);
}

GLuint Shader::getProgram() const {
  return program;
}

GLint Shader::getUniformLocation(const std::string& name) const {
  auto it = uniformCache.find(name);
  if (it != uniformCache.end()) return it->second;
  GLint loc = glGetUniformLocation(program, name.c_str());
  uniformCache[name] = loc;
  return loc;
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
  glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
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
