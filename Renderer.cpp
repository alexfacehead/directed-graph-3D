#include "Mesh.h"
#include "Material.h"
#include "Camera.h"
#include <memory>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Mesh& mesh, const Material& material, const Camera& camera);

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

void Renderer::render(const Mesh& mesh, const Material& material, const Camera& camera) {
    material.use(); // Use the material (set shader uniforms for material properties).
    // Set shader uniforms for camera properties (e.g., view and projection matrices) here.
    // This will depend on your shader implementation.
    mesh.draw();    // Draw the mesh.
}
