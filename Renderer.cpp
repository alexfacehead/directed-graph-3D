#include "Mesh.h"
#include "Material.h"
#include <memory>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Mesh& mesh, const Material& material);

private:
    // Initialize shaders, load resources, and set up any required OpenGL state.
    void initialize();
};

Renderer::Renderer() {
    initialize();
}

Renderer::~Renderer() {
    // Clean up any resources and OpenGL state here.
}

void Renderer::initialize() {
    // Initialize shaders, load resources, and set up any required OpenGL state.
}

void Renderer::render(const Mesh& mesh, const Material& material) {
    material.use(); // Use the material (set shader uniforms for material properties).
    mesh.draw();    // Draw the mesh.
}
