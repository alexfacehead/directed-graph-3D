// Hypergraph.cpp
#include "Hypergraph.h"
#include "iostream"
#include "glm/glm.hpp"
#include "Mesh.h"
#include <glm/glm.hpp>
#include <vector>



// constructor
// @param numVertices : the size of the graph
Hypergraph::Hypergraph(size_t numVertices)
    : numVertices(numVertices),
      adjacencyLists(numVertices, std::list<std::list<size_t>>()) {}

// Getter method for all the unique edges
const std::vector<std::list<size_t>>& Hypergraph::getAllHyperedges() const {
    return allHyperedges;
}


// Getter method for all the unique vertices and indices
// Remove the const Hypergraph& hypergraph parameter
LineData Hypergraph::getLineData() const {
    std::vector<glm::vec3> lineVertices;
    std::vector<unsigned int> lineIndices;

    size_t currentIndex = 0;

    // Replace hypergraph.getAllHyperedges() with getAllHyperedges()
    for (const auto& hyperedge : getAllHyperedges()) {
        // Iterate through the vertices in the hyperedge
        for (size_t vertexIndex : hyperedge) {
            // Replace hypergraph.indexToPosition(vertexIndex) with indexToPosition(vertexIndex)
            lineVertices.push_back(indexToPosition(vertexIndex));

            // Add the current index to lineIndices
            lineIndices.push_back(currentIndex);
            currentIndex++;
        }
    }

    return LineData{lineVertices, lineIndices};
}

// Converts an index to a point on a 3D grid
glm::vec3 Hypergraph::indexToPosition(size_t index) const {
    const float gridSpacing = 1.0f; // Adjust this value as needed

    // Calculate the x, y, and z coordinates of the vertex based on its index
    float x = (index % 10) * gridSpacing;
    float y = ((index / 10) % 10) * gridSpacing;
    float z = (index / 100) * gridSpacing;

    return glm::vec3(x, y, z);
}

void Hypergraph::addHyperedge(const std::list<size_t>& vertices) {
    for (size_t vertex : vertices) {
        if (vertex >= numVertices) {
            std::cerr << "Invalid vertex index: " << vertex << ", total number of vertices: " << numVertices << ", hyperedge: ";
            for (size_t v : vertices) {
                std::cerr << v << ' ';
            }
            std::cerr << std::endl;
            return;
        }
        std::cout << "Adding vertex " << vertex << " to adjacency list of hyperedge: ";
        for (size_t v : vertices) {
            std::cout << v << ' ';
        }
        std::cout << std::endl;
        adjacencyLists[vertex].push_back(vertices);
    }
    allHyperedges.push_back(vertices);
}



// Returns the list of hyperedges
const std::list<std::list<size_t>>& Hypergraph::getAdjacentHyperedges(size_t vertex) const {
    if (vertex >= numVertices) {
        std::cerr << "Invalid vertex index" << std::endl;
        static std::list<std::list<size_t>> emptyList;
        return emptyList;
    }
    return adjacencyLists[vertex];
}

size_t Hypergraph::getNumVertices() const {
    return numVertices;
}

// Print method
void Hypergraph::printHypergraph() const {
    for (size_t i = 0; i < numVertices; ++i) {
        std::cout << "Vertex " << i << " hyperedges: ";
        for (const auto &hyperedge : adjacencyLists[i]) {
            std::cout << "{ ";
            for (size_t v : hyperedge) {
                std::cout << v << ' ';
            }
            std::cout << "} ";
        }
        std::cout << std::endl;
    }
}
