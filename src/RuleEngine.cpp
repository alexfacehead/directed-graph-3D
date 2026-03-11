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

// Reservoir sampling: find random {{x,y},{x,z}} match in O(edges) time, O(V) memory.
// Builds vertex->edge index map, then streams through pairs picking uniformly at random.
bool RuleEngine::findSharedPair(const Hypergraph& g, std::mt19937& rng,
                                uint32_t& x, uint32_t& y, uint32_t& z,
                                size_t& edgeA, size_t& edgeB) {
    size_t ne = g.getNumEdges();
    if (ne < 2) return false;

    // Build vertex -> edge index lists (flat vectors, no hash tables)
    size_t nv = g.getNumVertices();
    // Count edges per vertex first (for reserve)
    std::vector<uint32_t> counts(nv, 0);
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = g.edgeA(i), b = g.edgeB(i);
        if (a < nv) counts[a]++;
        if (b < nv && b != a) counts[b]++;
    }

    // Build adjacency using offsets (CSR-lite)
    std::vector<uint32_t> offsets(nv + 1, 0);
    for (size_t i = 0; i < nv; ++i) offsets[i + 1] = offsets[i] + counts[i];
    std::vector<uint32_t> adjEdges(offsets[nv]);
    std::vector<uint32_t> pos(nv, 0); // write cursor per vertex
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = g.edgeA(i), b = g.edgeB(i);
        if (a < nv) adjEdges[offsets[a] + pos[a]++] = static_cast<uint32_t>(i);
        if (b < nv && b != a) adjEdges[offsets[b] + pos[b]++] = static_cast<uint32_t>(i);
    }

    // Reservoir sampling over all valid (edgeA, edgeB, x, y, z) matches
    size_t matchCount = 0;
    bool found = false;

    for (uint32_t vtx = 0; vtx < nv; ++vtx) {
        uint32_t start = offsets[vtx], end = offsets[vtx + 1];
        if (end - start < 2) continue;

        for (uint32_t ai = start; ai < end; ++ai) {
            for (uint32_t bi = ai + 1; bi < end; ++bi) {
                uint32_t iA = adjEdges[ai], iB = adjEdges[bi];
                uint32_t a0 = g.edgeA(iA), a1 = g.edgeB(iA);
                uint32_t b0 = g.edgeA(iB), b1 = g.edgeB(iB);

                uint32_t yy = (a0 == vtx) ? a1 : a0;
                uint32_t zz = (b0 == vtx) ? b1 : b0;

                matchCount++;
                // Reservoir sampling: keep with probability 1/matchCount
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
