#pragma once

#include "Mesh.h"
#include "Material.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void render(const Mesh& mesh, const Material& material);

private:
    void initialize();
};
