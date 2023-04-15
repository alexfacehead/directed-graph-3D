// Hypergraph.h
#pragma once
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>


struct LineData {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
};

class Hypergraph {
public:
    Hypergraph(size_t numVertices);

    void addHyperedge(const std::list<size_t>& vertices);

    const std::list<std::list<size_t>>& getAdjacentHyperedges(size_t vertex) const;

    size_t getNumVertices() const;

    void printHypergraph() const;

    glm::vec3 indexToPosition(size_t index) const;

    LineData getLineData() const;

    const std::vector<std::list<size_t>>& getAllHyperedges() const;

private:
    size_t numVertices;
    std::vector<std::list<std::list<size_t>>> adjacencyLists;
    std::vector<std::list<size_t>> allHyperedges;
};