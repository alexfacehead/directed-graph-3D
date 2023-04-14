#include <GL/glew.h>
#include <glm/glm.hpp>

class Material {
public:
    Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess);
    ~Material();

    void use() const;

private:
    glm::vec3 m_ambient;
    glm::vec3 m_diffuse;
    glm::vec3 m_specular;
    float m_shininess;
};

Material::Material(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess)
    : m_ambient(ambient), m_diffuse(diffuse), m_specular(specular), m_shininess(shininess) {}

Material::~Material() {}

void Material::use() const {
    // Set shader uniforms for the material properties here.
    // This will depend on your shader implementation.
}
