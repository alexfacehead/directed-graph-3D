// ForceLayout.cpp
//
// Two-part layout: multilevel coarsening for macro structure, local-only
// forces for per-frame refinement. The coarsening (Walshaw-style) collapses
// the graph into smaller versions, lays out the smallest, and projects back.
// The incremental step only repels direct neighbors, not all pairs, which
// prevents the isotropic pressure that makes standard spring-embedders
// produce spheres regardless of topology.
#include "ForceLayout.h"
#include <cmath>
#include <algorithm>
#include <cstring>

void ForceLayout::step(Hypergraph& graph) {
    const size_t n = graph.getNumVertices();
    const size_t ne = graph.getNumEdges();
    if (n == 0) return;

    // Resize persistent buffers (no realloc if already big enough)
    forces.resize(n);
    degrees.resize(n);
    std::memset(forces.data(), 0, n * sizeof(glm::vec3));
    std::memset(degrees.data(), 0, n * sizeof(uint32_t));

    const uint32_t* ed = graph.edgeData();
    glm::vec3* pos = graph.positionData();
    const float invN = 1.0f / static_cast<float>(n);

    // Pass 1: compute degrees from edge list
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = ed[2 * i], b = ed[2 * i + 1];
        if (a != b) { degrees[a]++; degrees[b]++; }
    }

    // Pass 2: combined repulsion + attraction per edge (single pass, no hash sets)
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = ed[2 * i], b = ed[2 * i + 1];
        if (a == b) continue;

        glm::vec3 diff = pos[a] - pos[b];
        float dist2 = glm::dot(diff, diff);
        if (dist2 < 1e-4f) dist2 = 1e-4f;

        // Fast inverse sqrt: one sqrt for both repulsion and attraction
        float dist = std::sqrt(dist2);
        float invDist = 1.0f / dist;

        // Degree-weighted repulsion: pushes apart
        float degA = static_cast<float>(degrees[a]) + 1.0f;
        float degB = static_cast<float>(degrees[b]) + 1.0f;
        float rep = repulsionStrength * degA * degB * invN / dist2;
        if (rep > maxForce) rep = maxForce;

        // LinLog attraction: pulls together
        float att = attractionStrength * std::log1p(dist / restLength);

        // Net force along edge direction
        float net = rep - att;
        glm::vec3 f = diff * (invDist * net);
        forces[a] += f;
        forces[b] -= f;
    }

    // Pass 3: integrate velocity + position
    for (size_t i = 0; i < n; ++i) {
        glm::vec3& vel = graph.getVelocity(i);
        vel = (vel + forces[i] * dt) * damping;
        pos[i] += vel * dt;
    }
}

void ForceLayout::computeInitialLayout(Hypergraph& graph, size_t totalVertices) {
    const size_t n = graph.getNumVertices();
    if (n < 4) return;
    if (totalVertices == 0) totalVertices = n;

    const size_t ne = graph.getNumEdges();
    const uint32_t* ed = graph.edgeData();

    // Build edge pairs from flat array
    std::vector<std::pair<uint32_t, uint32_t>> edgePairs;
    edgePairs.reserve(ne);
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = ed[2 * i], b = ed[2 * i + 1];
        if (a != b) edgePairs.push_back({a, b});
    }

    // Build coarsening hierarchy
    std::vector<CoarseLevel> levels;

    // Level 0 = original graph
    {
        CoarseLevel l0;
        l0.positions.resize(n);
        const glm::vec3* pos = graph.positionData();
        std::memcpy(l0.positions.data(), pos, n * sizeof(glm::vec3));
        l0.edges = edgePairs;
        l0.fineToCoarse.resize(n);
        for (uint32_t i = 0; i < n; ++i) l0.fineToCoarse[i] = i;
        levels.push_back(std::move(l0));
    }

    // Coarsen: greedy adjacent-vertex matching
    while (levels.back().positions.size() > 20) {
        auto& prev = levels.back();
        const uint32_t pn = static_cast<uint32_t>(prev.positions.size());

        // Build flat adjacency
        std::vector<std::vector<uint32_t>> adj(pn);
        for (auto& [a, b] : prev.edges) {
            if (a < pn && b < pn && a != b) {
                adj[a].push_back(b);
                adj[b].push_back(a);
            }
        }

        // Greedy matching
        std::vector<int32_t> match(pn, -1);
        for (uint32_t i = 0; i < pn; ++i) {
            if (match[i] >= 0) continue;
            for (uint32_t nb : adj[i]) {
                if (match[nb] < 0 && nb != i) {
                    match[i] = static_cast<int32_t>(nb);
                    match[nb] = static_cast<int32_t>(i);
                    break;
                }
            }
        }

        // Assign coarse IDs
        std::vector<uint32_t> fineToCoarse(pn, UINT32_MAX);
        uint32_t coarseCount = 0;
        std::vector<glm::vec3> coarsePos;
        coarsePos.reserve(pn / 2 + 1);

        for (uint32_t i = 0; i < pn; ++i) {
            if (fineToCoarse[i] != UINT32_MAX) continue;
            uint32_t ci = coarseCount++;
            fineToCoarse[i] = ci;

            if (match[i] >= 0) {
                uint32_t j = static_cast<uint32_t>(match[i]);
                fineToCoarse[j] = ci;
                coarsePos.push_back((prev.positions[i] + prev.positions[j]) * 0.5f);
            } else {
                coarsePos.push_back(prev.positions[i]);
            }
        }

        if (coarseCount >= pn * 9 / 10) break;

        // Deduplicate coarse edges using sorted vector instead of hash set
        std::vector<std::pair<uint32_t, uint32_t>> coarseEdges;
        coarseEdges.reserve(prev.edges.size());
        for (auto& [a, b] : prev.edges) {
            uint32_t ca = fineToCoarse[a], cb = fineToCoarse[b];
            if (ca != cb) {
                coarseEdges.push_back({std::min(ca, cb), std::max(ca, cb)});
            }
        }
        std::sort(coarseEdges.begin(), coarseEdges.end());
        coarseEdges.erase(std::unique(coarseEdges.begin(), coarseEdges.end()),
                          coarseEdges.end());

        CoarseLevel cl;
        cl.positions = std::move(coarsePos);
        cl.edges = std::move(coarseEdges);
        cl.fineToCoarse = std::move(fineToCoarse);
        levels.push_back(std::move(cl));
    }

    if (levels.size() < 2) return;

    // Scale iterations based on graph size to prevent freezes on large graphs
    size_t coarsestIters, fineIters, finestIters;
    if (totalVertices > 5000) {
        coarsestIters = 80;
        fineIters = 30;
        finestIters = 50;
    } else if (totalVertices > 1000) {
        coarsestIters = 150;
        fineIters = 50;
        finestIters = 80;
    } else {
        coarsestIters = 300;
        fineIters = 80;
        finestIters = 150;
    }

    // Refine coarsest level starting from inherited positions (midpoints from
    // coarsening), NOT random. This preserves the current layout's orientation
    // and prevents visual "snapping" when coarsening triggers.
    auto& coarsest = levels.back();
    layoutLevel(coarsest, coarsestIters);

    // Project back up through levels
    std::uniform_real_distribution<float> jitter(-0.05f, 0.05f);
    for (int l = static_cast<int>(levels.size()) - 2; l >= 0; --l) {
        auto& fineLevel = levels[l];
        auto& coarseLevel = levels[l + 1];
        const size_t fn = fineLevel.positions.size();

        for (size_t i = 0; i < fn; ++i) {
            uint32_t c = coarseLevel.fineToCoarse[i];
            if (c < coarseLevel.positions.size()) {
                fineLevel.positions[i] = coarseLevel.positions[c]
                    + glm::vec3(jitter(rng), jitter(rng), jitter(rng));
            }
        }

        size_t iters = (l == 0) ? finestIters : fineIters;
        layoutLevel(fineLevel, iters);
    }

    // Copy results back
    auto& finest = levels[0];
    glm::vec3* pos = graph.positionData();
    std::memcpy(pos, finest.positions.data(), n * sizeof(glm::vec3));
    for (size_t i = 0; i < n; ++i) {
        graph.getVelocity(i) = glm::vec3(0.0f);
    }
}

void ForceLayout::layoutLevel(CoarseLevel& level, size_t iterations) {
    const size_t n = level.positions.size();
    if (n < 2) return;

    const float k = std::sqrt(4.0f / static_cast<float>(n));
    const float k2 = k * k;
    const float invK = 1.0f / k;
    float temp = 1.0f;

    std::vector<glm::vec3> f(n);

    for (size_t iter = 0; iter < iterations; ++iter) {
        std::memset(f.data(), 0, n * sizeof(glm::vec3));

        // Repulsion: k^2 / dist (Fruchterman-Reingold)
        for (size_t i = 0; i < n; ++i) {
            const glm::vec3 pi = level.positions[i];
            for (size_t j = i + 1; j < n; ++j) {
                glm::vec3 diff = pi - level.positions[j];
                float dist2 = glm::dot(diff, diff);
                if (dist2 < 1e-6f) dist2 = 1e-6f;
                float dist = std::sqrt(dist2);
                float fmag = k2 / dist;
                if (fmag > maxForce) fmag = maxForce;
                glm::vec3 fv = diff * (fmag / dist);
                f[i] += fv;
                f[j] -= fv;
            }
        }

        // Attraction: dist^2 / k
        for (auto& [a, b] : level.edges) {
            glm::vec3 diff = level.positions[b] - level.positions[a];
            float dist2 = glm::dot(diff, diff);
            if (dist2 < 1e-6f) continue;
            float dist = std::sqrt(dist2);
            float fmag = dist * dist * invK;
            glm::vec3 fv = diff * (fmag / dist);
            f[a] += fv;
            f[b] -= fv;
        }

        // Apply with simulated annealing
        for (size_t i = 0; i < n; ++i) {
            float flen2 = glm::dot(f[i], f[i]);
            if (flen2 > 1e-6f) {
                float flen = std::sqrt(flen2);
                float disp = std::min(flen, temp);
                level.positions[i] += f[i] * (disp / flen);
            }
        }
        temp *= 0.97f;
    }
}
