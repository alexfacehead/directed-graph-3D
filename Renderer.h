// Renderer.h
#pragma once

#include <GL/glew.h>
#include "Mesh.h"
#include "Material.h"
#include "Camera.h" // Include the Camera header

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Mesh& mesh, const Material& material, const Camera& camera);

private:
    void initialize();
};
