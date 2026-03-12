// ForceLayout.h
#pragma once
#include "Hypergraph.h"
#include <vector>
#include <random>

class ForceLayout {
public:
    float repulsionStrength = 1.0f;
    float attractionStrength = 0.3f;
    float restLength = 0.25f;
    float damping = 0.85f;
    float maxForce = 2.0f;
    float dt = 0.016f;

    void step(Hypergraph& graph);
    void computeInitialLayout(Hypergraph& graph, size_t totalVertices = 0);

private:
    // Persistent buffers — allocated once, reused every frame (zero alloc per step)
    std::vector<glm::vec3> forces;
    std::vector<uint32_t> degrees;
    std::mt19937 rng{123};

    struct CoarseLevel {
        std::vector<glm::vec3> positions;
        std::vector<std::pair<uint32_t, uint32_t>> edges;
        std::vector<uint32_t> fineToCoarse;
    };

    void layoutLevel(CoarseLevel& level, size_t iterations);
};
