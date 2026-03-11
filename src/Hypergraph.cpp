// Hypergraph.cpp
//
// Flat packed edge storage: edges are contiguous uint32_t pairs in memory.
// No linked lists, no per-edge heap allocation. Edge removal is O(1) via
// swap-and-pop. This layout is cache-friendly for the force simulation
// which iterates all edges every frame.
#include "Hypergraph.h"

Hypergraph::Hypergraph(size_t numVertices)
    : numVerts(numVertices),
      positions(numVertices, glm::vec3(0.0f)),
      velocities(numVertices, glm::vec3(0.0f)) {}

size_t Hypergraph::addVertex(glm::vec3 initialPosition) {
    size_t idx = numVerts++;
    positions.push_back(initialPosition);
    velocities.emplace_back(0.0f);
    return idx;
}

void Hypergraph::addEdge(uint32_t a, uint32_t b) {
    edges.push_back(a);
    edges.push_back(b);
    numEdges++;
}

void Hypergraph::removeEdge(size_t index) {
    if (index >= numEdges) return;
    size_t last = numEdges - 1;
    if (index != last) {
        edges[2 * index]     = edges[2 * last];
        edges[2 * index + 1] = edges[2 * last + 1];
    }
    numEdges--;
    edges.resize(2 * numEdges);
}

void Hypergraph::clear() {
    numVerts = 0;
    numEdges = 0;
    positions.clear();
    velocities.clear();
    edges.clear();
}
