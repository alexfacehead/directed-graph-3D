#include "Material.h"

Material::Material(const Shader& shader, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess)
    : m_shader(shader), m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {}

Material::~Material() {}

void Material::use() const {
    // Set shader uniforms for the material properties here.
    // This will depend on your shader implementation.
}
