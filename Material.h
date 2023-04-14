#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Material {
public:
    Material(const Shader& shader, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess);
    ~Material();

    void use() const;

private:
    Shader m_shader;
    glm::vec3 m_ambient;
    glm::vec3 m_diffuse;
    glm::vec3 m_specular;
    float m_shininess;
};
