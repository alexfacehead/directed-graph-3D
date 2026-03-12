// RuleEngine.cpp
//
// Four Wolfram Physics-style rewriting rules. All match the same pattern
// (two edges sharing a vertex) and replace it with a new structure plus a
// fresh vertex. Pattern matching uses CSR adjacency + reservoir sampling
// to pick a uniformly random match in one pass without storing all matches.
#include "RuleEngine.h"
#include <cmath>

static glm::vec3 randomOffset(std::mt19937& rng) {
    std::uniform_real_distribution<float> dist(-0.01f, 0.01f);
    return glm::vec3(dist(rng), dist(rng), dist(rng));
}

RuleEngine::RuleEngine() {
    rules.push_back({"Branching",
        "{{x,y},{x,z}} -> {{x,y},{x,w},{y,w},{z,w}}", rule1});
    rules.push_back({"Looping",
        "{{x,y},{x,z}} -> {{x,y},{y,z},{x,w}}", rule2});
    rules.push_back({"Spreading",
        "{{x,y},{x,z}} -> {{w,y},{w,z},{x,w}}", rule3});
    rules.push_back({"Knitting",
        "{{x,y},{x,z}} -> {{y,z},{x,w},{y,w}}", rule4});
    rules.push_back({"Chain",
        "{{x,y},{x,z}} -> {{x,w},{w,y},{w,z},{y,z}}", rule5});
    rules.push_back({"Crystalline",
        "{{x,y},{x,z}} -> {{x,y},{x,z},{y,w},{z,w},{x,w}}", rule6});
}

bool RuleEngine::applyStep(Hypergraph& graph) {
    if (currentRule < rules.size()) {
        return rules[currentRule].apply(graph, rng);
    }
    return false;
}

void RuleEngine::seedGraph(Hypergraph& graph, size_t /*ruleIndex*/) {
    graph.clear();
    seedSelfLoop(graph);
}

void RuleEngine::seedSelfLoop(Hypergraph& g) {
    g.addVertex(glm::vec3(0.0f));
    g.addEdge(0, 0);
    g.addEdge(0, 0);
}

size_t RuleEngine::getNumRules() const { return rules.size(); }
const std::string& RuleEngine::getRuleName(size_t i) const { return rules[i].name; }
const std::string& RuleEngine::getRuleDescription(size_t i) const { return rules[i].description; }
size_t RuleEngine::getCurrentRule() const { return currentRule; }
void RuleEngine::setCurrentRule(size_t index) { currentRule = index; }

// Reservoir sampling: find random {{x,y},{x,z}} match using persistent adjacency.
// No CSR rebuild — reads getAdj() directly. O(sum of degree^2) worst case but
// typically much faster than the old O(V+E) rebuild approach.
bool RuleEngine::findSharedPair(const Hypergraph& g, std::mt19937& rng,
                                uint32_t& x, uint32_t& y, uint32_t& z,
                                size_t& edgeA, size_t& edgeB) {
    size_t ne = g.getNumEdges();
    if (ne < 2) return false;

    size_t nv = g.getNumVertices();
    size_t matchCount = 0;
    bool found = false;

    for (uint32_t vtx = 0; vtx < nv; ++vtx) {
        const auto& adj = g.getAdj(vtx);
        if (adj.size() < 2) continue;

        for (size_t ai = 0; ai < adj.size(); ++ai) {
            for (size_t bi = ai + 1; bi < adj.size(); ++bi) {
                uint32_t iA = adj[ai], iB = adj[bi];
                if (iA >= ne || iB >= ne) continue; // safety check

                uint32_t a0 = g.edgeA(iA), a1 = g.edgeB(iA);
                uint32_t b0 = g.edgeA(iB), b1 = g.edgeB(iB);

                uint32_t yy = (a0 == vtx) ? a1 : a0;
                uint32_t zz = (b0 == vtx) ? b1 : b0;

                matchCount++;
                std::uniform_int_distribution<size_t> pick(0, matchCount - 1);
                if (pick(rng) == 0) {
                    x = vtx; y = yy; z = zz;
                    edgeA = iA; edgeB = iB;
                    found = true;
                }
            }
        }
    }
    return found;
}

bool RuleEngine::rule1(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    glm::vec3 mid = (g.getPosition(x) + g.getPosition(y) + g.getPosition(z)) / 3.0f;
    uint32_t w = static_cast<uint32_t>(g.addVertex(mid + randomOffset(rng)));

    size_t hi = std::max(eA, eB), lo = std::min(eA, eB);
    g.removeEdge(hi);
    g.removeEdge(lo);

    g.addEdge(x, y);
    g.addEdge(x, w);
    g.addEdge(y, w);
    g.addEdge(z, w);
    return true;
}

bool RuleEngine::rule2(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    uint32_t w = static_cast<uint32_t>(g.addVertex(g.getPosition(x) + randomOffset(rng)));

    size_t hi = std::max(eA, eB), lo = std::min(eA, eB);
    g.removeEdge(hi);
    g.removeEdge(lo);

    g.addEdge(x, y);
    g.addEdge(y, z);
    g.addEdge(x, w);
    return true;
}

bool RuleEngine::rule3(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    uint32_t w = static_cast<uint32_t>(g.addVertex(g.getPosition(x) + randomOffset(rng)));

    size_t hi = std::max(eA, eB), lo = std::min(eA, eB);
    g.removeEdge(hi);
    g.removeEdge(lo);

    g.addEdge(w, y);
    g.addEdge(w, z);
    g.addEdge(x, w);
    return true;
}

bool RuleEngine::rule4(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    uint32_t w = static_cast<uint32_t>(g.addVertex(g.getPosition(y) + randomOffset(rng)));

    size_t hi = std::max(eA, eB), lo = std::min(eA, eB);
    g.removeEdge(hi);
    g.removeEdge(lo);

    g.addEdge(y, z);
    g.addEdge(x, w);
    g.addEdge(y, w);
    return true;
}

// Chain: {{x,y},{x,z}} -> {{x,w},{w,y},{w,z},{y,z}}
// Creates filamentary structures with cross-links. The new vertex w replaces
// x as the hub, and y-z get directly connected. Produces long spindly chains.
bool RuleEngine::rule5(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    glm::vec3 mid = (g.getPosition(x) + g.getPosition(y) + g.getPosition(z)) / 3.0f;
    uint32_t w = static_cast<uint32_t>(g.addVertex(mid + randomOffset(rng)));

    size_t hi = std::max(eA, eB), lo = std::min(eA, eB);
    g.removeEdge(hi);
    g.removeEdge(lo);

    g.addEdge(x, w);
    g.addEdge(w, y);
    g.addEdge(w, z);
    g.addEdge(y, z);
    return true;
}

// Crystalline: {{x,y},{x,z}} -> {{x,y},{x,z},{y,w},{z,w},{x,w}}
// Keeps original edges intact and adds three new edges to a fresh vertex.
// Produces very dense, lattice-like growth with high connectivity.
bool RuleEngine::rule6(Hypergraph& g, std::mt19937& rng) {
    uint32_t x, y, z; size_t eA, eB;
    if (!findSharedPair(g, rng, x, y, z, eA, eB)) return false;

    glm::vec3 mid = (g.getPosition(x) + g.getPosition(y) + g.getPosition(z)) / 3.0f;
    uint32_t w = static_cast<uint32_t>(g.addVertex(mid + randomOffset(rng)));

    // Don't remove matched edges — keep them
    g.addEdge(y, w);
    g.addEdge(z, w);
    g.addEdge(x, w);
    return true;
}
