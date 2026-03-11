// shader.cpp
#include "Shader.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader Shader::fromSource(const char* vertexSource, const char* fragmentSource) {
  Shader s;
  GLuint vs = compileShader(vertexSource, GL_VERTEX_SHADER, "VERTEX");
  GLuint fs = compileShader(fragmentSource, GL_FRAGMENT_SHADER, "FRAGMENT");

  s.program = glCreateProgram();
  glAttachShader(s.program, vs);
  glAttachShader(s.program, fs);
  glLinkProgram(s.program);
  checkShaderErrors(s.program, "PROGRAM", GL_LINK_STATUS);

  glDeleteShader(vs);
  glDeleteShader(fs);
  return s;
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

GLuint Shader::compileShader(const char* source, GLenum shaderType, const char* label) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  checkShaderErrors(shader, label, GL_COMPILE_STATUS);
  return shader;
}

void Shader::checkShaderErrors(GLuint shader, const char* label, GLenum statusType) {
    GLint success;
    if (statusType == GL_COMPILE_STATUS) {
        glGetShaderiv(shader, statusType, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::COMPILATION::" << label << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, statusType, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER::LINKING::" << label << "\n" << infoLog << std::endl;
        }
    }
}
