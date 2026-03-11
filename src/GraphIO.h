// GraphIO.h
#pragma once
#include "Hypergraph.h"
#include <string>

class GraphIO {
public:
    static bool save(const Hypergraph& graph, size_t ruleIndex, const std::string& filepath);
    static bool load(Hypergraph& graph, size_t& ruleIndex, const std::string& filepath);
    static std::string generateFilepath();
};
