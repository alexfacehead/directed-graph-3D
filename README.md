# Hypergraph 3D

Real-time 3D visualization of Wolfram Physics-style hypergraph rewriting.

Starts from a single self-looping vertex, applies a rewriting rule every frame, and grows into an organic network structure. The layout algorithm is designed to reveal the topology of the graph rather than squashing it into a ball.

![Branching rule at 10,000 vertices](https://img.shields.io/badge/C%2B%2B-17-blue) ![OpenGL 3.3](https://img.shields.io/badge/OpenGL-3.3-green)

## Background

Stephen Wolfram's Physics Project explores whether the universe might be a giant hypergraph that evolves by simple rewriting rules. You pick a pattern, find it somewhere in the graph, delete the matched edges, and replace them with a new structure (including a new vertex). Doing this thousands of times produces surprisingly complex shapes from trivial seeds.

This project implements four of those rules and visualizes the growing graph in real time. The tricky part is the layout: a naive spring-embedder will always produce a sphere, because all-pairs repulsion is isotropic. Getting the distinctive Wolfram look (dense clusters connected by thin bridges) required a multilevel coarsening layout algorithm and local-only physics.

## How it works

### The rewriting rules

All four rules match the same pattern: two edges that share a vertex, written `{{x,y},{x,z}}`. They differ in what they replace it with. Each step, the engine finds a random match, deletes those two edges, creates a new vertex `w`, and adds the replacement edges.

- **Branching**: `{{x,y},{x,z}}` becomes `{{x,y},{x,w},{y,w},{z,w}}`. The classic Wolfram rule. Produces dense, interconnected structures with visible clustering.
- **Looping**: `{{x,y},{x,z}}` becomes `{{x,y},{y,z},{x,w}}`. Creates cycles and loops. Tends to form long chains and ring-like structures.
- **Spreading**: `{{x,y},{x,z}}` becomes `{{w,y},{w,z},{x,w}}`. Inserts a hub between x and its neighbors. Produces radial, star-like patterns.
- **Knitting**: `{{x,y},{x,z}}` becomes `{{y,z},{x,w},{y,w}}`. Creates mesh-like interconnections. Produces beautiful branching tree structures.

The graph always starts from the same seed: a single vertex with two self-loops `{{0,0},{0,0}}`.

### The layout algorithm

Getting a good layout was the hardest part of this project. Here's what we tried and why most of it didn't work:

**What doesn't work: standard spring-embedder.** The textbook approach (every vertex repels every other vertex, connected vertices attract) always produces a sphere. It doesn't matter what force law you use or how you tune the parameters. The all-pairs repulsion creates uniform outward pressure in every direction, and the result is always a ball. We spent a long time tweaking 1/r^2, 1/r^3, cutoff radii, and attraction models before figuring this out.

**What works: multilevel coarsening + local-only forces.**

The layout has two parts:

1. **Multilevel coarsening** (runs periodically as the graph grows). This is based on Walshaw's algorithm, the same family of techniques Mathematica uses internally. It works by:
   - Repeatedly collapsing pairs of adjacent vertices into single vertices, building a hierarchy of progressively smaller graphs
   - Laying out the smallest graph (about 20 vertices) using a standard Fruchterman-Reingold force simulation
   - Projecting positions back up through each level, adding small jitter, and refining with more force simulation at each level
   - This captures the large-scale structure (which clusters exist, how they connect) without getting trapped in local minima

2. **Local-only incremental physics** (runs every frame). Connected vertices repel each other (to prevent overlap) and attract each other (to keep edges short). The key difference from a standard spring-embedder: repulsion only happens between directly connected vertices, not between all pairs. This means the physics resolves local overlaps without creating the global isotropic pressure that inflates everything into a sphere.

The repulsion is also degree-weighted: high-degree hub vertices push harder than leaf vertices. This helps separate clusters that share a hub node.

### Data structures

The graph is stored as flat packed arrays for cache locality:

- Edges are pairs of `uint32_t` vertex indices packed contiguously: `[a0,b0, a1,b1, a2,b2, ...]`
- Positions and velocities are contiguous `glm::vec3` arrays
- No linked lists, no hash tables in the hot path
- Edge removal is O(1) via swap-and-pop

The force simulation reuses pre-allocated buffers across frames (zero heap allocation per physics step). The rule engine uses CSR-style (compressed sparse row) adjacency and reservoir sampling to pick a random pattern match in a single pass.

### Performance

The incremental physics step is O(edges), not O(vertices^2). At 10,000 vertices with 20,000 edges, a single step takes well under a millisecond.

The multilevel coarsening is more expensive (the Fruchterman-Reingold refinement at each level is O(n^2) for that level), but it only triggers when the vertex count doubles and stops triggering above 2,000 vertices. After that, the local-only physics maintains the structure on its own.

Compiler flags: `-O3 -march=native -ffast-math -funroll-loops` with link-time optimization enabled.

## Controls

| Control | What it does |
|---------|-------------|
| Pause | Stops both rule application and physics |
| Interval | Frames between rule applications (1 = every frame, 30 = slow growth) |
| Zoom | Slider or scroll wheel |
| Orbit | Click and drag to rotate |
| Rule | Dropdown to switch rules. Reseeds the graph from scratch |
| Save | Writes graph state to `saves/` with a timestamp |
| Load | Type a path and click Load to restore |

## Building

### macOS (Homebrew)

```
brew install glfw glew glm cmake
cd directed-graph-3D
mkdir build && cd build
cmake ..
cmake --build .
./directed_graph
```

### Linux (apt)

```
sudo apt-get install libglfw3-dev libglew-dev libglm-dev cmake build-essential
cd directed-graph-3D
mkdir build && cd build
cmake ..
cmake --build .
./directed_graph
```

CMake fetches Dear ImGui v1.91.8 automatically via FetchContent.

## Project structure

```
src/
  Main.cpp          - Window setup, ImGui UI, render loop, mouse orbit
  Hypergraph.h/cpp  - Flat packed edge storage, O(1) add/remove
  RuleEngine.h/cpp  - Four Wolfram-style rewriting rules, reservoir sampling
  ForceLayout.h/cpp - Multilevel coarsening layout + local-only incremental physics
  Camera.h/cpp      - Spherical coordinate orbit camera with dirty-flag caching
  GraphIO.h/cpp     - Text-based save/load
  Mesh.h/cpp        - Dynamic OpenGL VBO/EBO with capacity doubling
  Shader.h/cpp      - GLSL loader with cached uniform locations
  Material.h/cpp    - Shader reference + material properties
  Renderer.h/cpp    - Sets MVP uniforms and draws
shaders/
  vertex_shader.glsl
  fragment_shader.glsl
```

## Dependencies

- OpenGL 3.3 core profile
- [GLFW](https://www.glfw.org/) for windowing
- [GLEW](http://glew.sourceforge.net/) for OpenGL extension loading
- [GLM](https://github.com/g-truc/glm) for vector/matrix math
- [Dear ImGui](https://github.com/ocornut/imgui) v1.91.8 (fetched automatically by CMake)

## Lessons learned

The biggest lesson: when a force-directed layout produces the wrong shape, the problem is usually architectural, not parametric. No amount of parameter tweaking will fix a fundamentally isotropic algorithm. The fix was structural: restrict repulsion to local neighborhoods and use multilevel coarsening to set the macro layout.
