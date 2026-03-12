// Hypergraph.cpp
//
// Flat packed edge storage with persistent adjacency lists. Edges are
// contiguous uint32_t pairs in memory. Each vertex maintains a list of
// edge indices touching it, incrementally updated on add/remove.
// Edge removal is O(degree) via swap-and-pop + adjacency fixup.
#include "Hypergraph.h"
#include <algorithm>

Hypergraph::Hypergraph(size_t numVertices)
    : numVerts(numVertices),
      positions(numVertices, glm::vec3(0.0f)),
      velocities(numVertices, glm::vec3(0.0f)),
      adjList(numVertices) {}

size_t Hypergraph::addVertex(glm::vec3 initialPosition) {
    size_t idx = numVerts++;
    positions.push_back(initialPosition);
    velocities.emplace_back(0.0f);
    adjList.emplace_back();
    return idx;
}

void Hypergraph::addEdge(uint32_t a, uint32_t b) {
    uint32_t edgeIdx = static_cast<uint32_t>(numEdges);
    edges.push_back(a);
    edges.push_back(b);
    numEdges++;

    // Update adjacency
    if (a < numVerts) adjList[a].push_back(edgeIdx);
    if (b < numVerts && b != a) adjList[b].push_back(edgeIdx);
}

void Hypergraph::removeEdge(size_t index) {
    if (index >= numEdges) return;

    uint32_t idx = static_cast<uint32_t>(index);
    uint32_t remA = edges[2 * index];
    uint32_t remB = edges[2 * index + 1];

    // Remove this edge from adjacency lists of its vertices
    if (remA < numVerts) removeEdgeFromAdj(remA, idx);
    if (remB < numVerts && remB != remA) removeEdgeFromAdj(remB, idx);

    size_t last = numEdges - 1;
    if (index != last) {
        uint32_t lastIdx = static_cast<uint32_t>(last);
        uint32_t swapA = edges[2 * last];
        uint32_t swapB = edges[2 * last + 1];

        // Swap last edge into removed slot
        edges[2 * index]     = swapA;
        edges[2 * index + 1] = swapB;

        // Update adjacency: the swapped edge changed index from lastIdx to idx
        if (swapA < numVerts) replaceEdgeInAdj(swapA, lastIdx, idx);
        if (swapB < numVerts && swapB != swapA) replaceEdgeInAdj(swapB, lastIdx, idx);
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
    adjList.clear();
}

void Hypergraph::removeEdgeFromAdj(uint32_t vertex, uint32_t edgeIdx) {
    auto& adj = adjList[vertex];
    for (size_t i = 0; i < adj.size(); ++i) {
        if (adj[i] == edgeIdx) {
            adj[i] = adj.back();
            adj.pop_back();
            return;
        }
    }
}

void Hypergraph::replaceEdgeInAdj(uint32_t vertex, uint32_t oldIdx, uint32_t newIdx) {
    auto& adj = adjList[vertex];
    for (size_t i = 0; i < adj.size(); ++i) {
        if (adj[i] == oldIdx) {
            adj[i] = newIdx;
            return;
        }
    }
}
