// Hypergraph.h
#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

// All edges are 2-vertex pairs, stored as flat packed uint32_t array
// for maximum cache locality and zero per-edge heap allocation.
class Hypergraph {
public:
    Hypergraph(size_t numVertices);

    size_t addVertex(glm::vec3 initialPosition);
    void addEdge(uint32_t a, uint32_t b);
    void removeEdge(size_t index);   // swap-and-pop O(1)
    void clear();

    size_t getNumVertices() const { return numVerts; }
    size_t getNumEdges() const { return numEdges; }

    // Edge access: edges packed as [a0,b0, a1,b1, ...]
    uint32_t edgeA(size_t i) const { return edges[2 * i]; }
    uint32_t edgeB(size_t i) const { return edges[2 * i + 1]; }
    const uint32_t* edgeData() const { return edges.data(); }

    // Position/velocity: contiguous arrays
    glm::vec3* positionData() { return positions.data(); }
    const glm::vec3* positionData() const { return positions.data(); }
    glm::vec3& getPosition(size_t v) { return positions[v]; }
    const glm::vec3& getPosition(size_t v) const { return positions[v]; }
    glm::vec3& getVelocity(size_t v) { return velocities[v]; }

private:
    size_t numVerts = 0;
    size_t numEdges = 0;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<uint32_t> edges; // packed pairs
};
