// RuleEngine.h
#pragma once
#include "Hypergraph.h"
#include <string>
#include <vector>
#include <functional>
#include <random>

struct Rule {
    std::string name;
    std::string description;
    std::function<bool(Hypergraph&, std::mt19937&)> apply;
};

class RuleEngine {
public:
    RuleEngine();

    bool applyStep(Hypergraph& graph);
    void seedGraph(Hypergraph& graph, size_t ruleIndex);

    size_t getNumRules() const;
    const std::string& getRuleName(size_t i) const;
    const std::string& getRuleDescription(size_t i) const;
    size_t getCurrentRule() const;
    void setCurrentRule(size_t index);

private:
    std::vector<Rule> rules;
    size_t currentRule = 0;
    std::mt19937 rng{42};

    static bool rule1(Hypergraph& g, std::mt19937& rng);
    static bool rule2(Hypergraph& g, std::mt19937& rng);
    static bool rule3(Hypergraph& g, std::mt19937& rng);
    static bool rule4(Hypergraph& g, std::mt19937& rng);

    static bool findSharedPair(const Hypergraph& g, std::mt19937& rng,
                               uint32_t& x, uint32_t& y, uint32_t& z,
                               size_t& edgeA, size_t& edgeB);

    static void seedSelfLoop(Hypergraph& g);
};
