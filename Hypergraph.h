// Hypergraph.h
#ifndef HYPERGRAPH_H
#define HYPERGRAPH_H

#include <vector>
#include <list>

class Hypergraph {
public:
    Hypergraph(size_t numVertices);

    void addHyperedge(const std::vector<size_t>& vertices);
    const std::list<std::vector<size_t>>& getAdjacentHyperedges(size_t vertex) const;
    size_t getNumVertices() const;

private:
    size_t numVertices;
    std::vector<std::list<std::vector<size_t>>> adjacencyLists;
};

#endif // HYPERGRAPH_H