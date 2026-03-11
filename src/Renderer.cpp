#include "Renderer.h"
#include <glm/glm.hpp>

Renderer::Renderer() {
    initialize();
}

Renderer::~Renderer() {}

void Renderer::initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void Renderer::render(const Mesh& mesh, const Material& material, const Camera& camera) {
    material.use();

    const Shader& shader = material.getShader();
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setMat4("view", camera.getViewMatrix());
    shader.setMat4("projection", camera.getProjectionMatrix());

    mesh.draw();
}
