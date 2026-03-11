#include "Material.h"

Material::Material(const Shader& shader, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess)
    : m_shader(shader), m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {}

Material::~Material() {}

void Material::use() const {
    m_shader.use();
}

const Shader& Material::getShader() const {
    return m_shader;
}
