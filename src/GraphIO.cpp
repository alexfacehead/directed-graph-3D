// GraphIO.cpp
#include "GraphIO.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>

bool GraphIO::save(const Hypergraph& graph, size_t ruleIndex, const std::string& filepath) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    size_t nv = graph.getNumVertices();
    size_t ne = graph.getNumEdges();

    out << "rule " << ruleIndex << "\n";
    out << "vertices " << nv << "\n";
    const glm::vec3* pos = graph.positionData();
    for (size_t i = 0; i < nv; ++i) {
        out << pos[i].x << " " << pos[i].y << " " << pos[i].z << "\n";
    }
    out << "edges " << ne << "\n";
    for (size_t i = 0; i < ne; ++i) {
        out << graph.edgeA(i) << " " << graph.edgeB(i) << "\n";
    }
    return true;
}

bool GraphIO::load(Hypergraph& graph, size_t& ruleIndex, const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::string token;
    size_t numVerts = 0, numEdges = 0;

    in >> token >> ruleIndex;
    in >> token >> numVerts;

    graph.clear();
    for (size_t i = 0; i < numVerts; ++i) {
        float x, y, z;
        in >> x >> y >> z;
        graph.addVertex(glm::vec3(x, y, z));
    }

    in >> token >> numEdges;
    for (size_t i = 0; i < numEdges; ++i) {
        uint32_t a, b;
        in >> a >> b;
        graph.addEdge(a, b);
    }
    return true;
}

std::string GraphIO::generateFilepath() {
    mkdir("saves", 0755);
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "saves/graph_%Y%m%d_%H%M%S.txt", std::localtime(&now));
    return std::string(buf);
}
