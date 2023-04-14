#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

class Hypergraph {
public:
    Hypergraph(size_t numVertices)
        : numVertices(numVertices),
          adjacencyLists(numVertices, std::list<std::vector<size_t>>()) {}

    void addHyperedge(const std::vector<size_t>& vertices) {
        for (size_t vertex : vertices) {
            if (vertex >= numVertices) {
                std::cerr << "Invalid vertex index" << std::endl;
                return;
            }
            adjacencyLists[vertex].push_back(vertices);
        }
    }

    const std::list<std::vector<size_t>>& getAdjacentHyperedges(size_t vertex) const {
        if (vertex >= numVertices) {
            std::cerr << "Invalid vertex index" << std::endl;
            static std::list<std::vector<size_t>> emptyList;
            return emptyList;
        }
        return adjacencyLists[vertex];
    }

    size_t getNumVertices() const {
        return numVertices;
    }

private:
    size_t numVertices;
    std::vector<std::list<std::vector<size_t>>> adjacencyLists;
};