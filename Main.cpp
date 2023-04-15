#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Hypergraph.h"

// Applies a rule to a hypergraph (not to be confused with addHyperEdge function)
void applyRule(Hypergraph& hypergraph) {
    // Iterate through all vertices in the hypergraph
    for (size_t vertex = 0; vertex < hypergraph.getNumVertices(); ++vertex) {
        // Get the adjacent hyperedges for the current vertex
        const std::list<std::list<size_t>>& adjacentHyperedges = hypergraph.getAdjacentHyperedges(vertex);

        // Iterate through the adjacent hyperedges
        for (const std::list<size_t>& hyperedge : adjacentHyperedges) {
            // Check if the hyperedge matches a specific pattern (e.g., a hyperedge with three vertices)
            if (hyperedge.size() == 3) {
                // Apply the rule by adding a new hyperedge connecting the first and last vertices of the matched hyperedge
                std::list<size_t> newHyperedge = {hyperedge.front(), hyperedge.back()};
                hypergraph.addHyperedge(newHyperedge);
            }
        }
    }
}

// Generates rendering data for a hypergraph
void generateHypergraphRenderingData(const Hypergraph& hypergraph,
                                     std::vector<Vertex>& nodeVertices,
                                     std::vector<GLuint>& nodeIndices,
                                     std::vector<Vertex>& lineVertices,
                                     std::vector<GLuint>& lineIndices) {
    // Clear the input vectors
    nodeVertices.clear();
    nodeIndices.clear();
    lineVertices.clear();
    lineIndices.clear();

    // Generate node vertices and indices
    size_t numVertices = hypergraph.getNumVertices();
    float nodeSize = 0.1f;

    for (size_t i = 0; i < numVertices; ++i) {
        // Calculate node position (e.g., arrange nodes in a circle)
        float angle = 2.0f * M_PI * float(i) / float(numVertices);
        float x = std::cos(angle);
        float y = std::sin(angle);

        // Generate vertices for a square node
        nodeVertices.push_back({{x - nodeSize, y - nodeSize, 0.0f}, {1.0f, 0.0f, 0.0f}});
        nodeVertices.push_back({{x + nodeSize, y - nodeSize, 0.0f}, {0.0f, 1.0f, 0.0f}});
        nodeVertices.push_back({{x + nodeSize, y + nodeSize, 0.0f}, {0.0f, 0.0f, 1.0f}});
        nodeVertices.push_back({{x - nodeSize, y + nodeSize, 0.0f}, {1.0f, 1.0f, 0.0f}});

        // Generate indices for a square node
        size_t baseIndex = 4 * i;
        nodeIndices.push_back(baseIndex + 0);
        nodeIndices.push_back(baseIndex + 1);
        nodeIndices.push_back(baseIndex + 2);
        nodeIndices.push_back(baseIndex + 0);
        nodeIndices.push_back(baseIndex + 2);
        nodeIndices.push_back(baseIndex + 3);
    }

    // Generate edge vertices and indices
    size_t lineVertexIndex = 0;
    for (size_t i = 0; i < numVertices; ++i) {
        const auto& adjacentHyperedges = hypergraph.getAdjacentHyperedges(i);

        for (const auto& hyperedge : adjacentHyperedges) {
            for (const auto& targetVertex : hyperedge) {
                // Only generate line data for half of the pairs (avoid duplicate lines)
                if (targetVertex > i) {
                    // Use node positions as line endpoints
                    const GLfloat* sourcePos = nodeVertices[4 * i].position;
                    const GLfloat* targetPos = nodeVertices[4 * targetVertex].position;

                    // Create and add source vertex to lineVertices
                    Vertex sourceVertex;
                    sourceVertex.position[0] = sourcePos[0];
                    sourceVertex.position[1] = sourcePos[1];
                    sourceVertex.position[2] = sourcePos[2];
                    sourceVertex.color[0] = 1.0f;
                    sourceVertex.color[1] = 1.0f;
                    sourceVertex.color[2] = 1.0f;
                    lineVertices.push_back(sourceVertex);

                    // Create and add target vertex to lineVertices
                    Vertex targetVertexObj;
                    targetVertexObj.position[0] = targetPos[0];
                    targetVertexObj.position[1] = targetPos[1];
                    targetVertexObj.position[2] = targetPos[2];
                    targetVertexObj.color[0] = 1.0f;
                    targetVertexObj.color[1] = 1.0f;
                    targetVertexObj.color[2] = 1.0f;
                    lineVertices.push_back(targetVertexObj);

                    // Generate indices for the line
                    lineIndices.push_back(lineVertexIndex + 0);
                    lineIndices.push_back(lineVertexIndex + 1);

                    lineVertexIndex += 2;
                }
            }
        }
    }
}

// Sets up hyperedges according to the simplest rules of the Wolfram 
// Physics Project, supposedly!
void addHyperedgesExample(Hypergraph& hypergraph) {
    size_t numVertices = hypergraph.getNumVertices();
    
    for (size_t vertex = 0; vertex < numVertices; ++vertex) {
        std::list<size_t> hyperedgeVertices = {vertex, (vertex + 1) % numVertices, (vertex + 2) % numVertices};
        hypergraph.addHyperedge(hyperedgeVertices);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
    // Initialization failed
    return -1;
    }

    // Create a window with GLFW
    GLFWwindow *window = glfwCreateWindow(1024, 768, "My Window", NULL, NULL);
    if (!window) {
    // Window creation failed
    glfwTerminate();
    return -1;
    }

    // Make the window's OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewInit();

    // Load shader files
    Shader shader("./vertex_shader.glsl", "./fragment_shader.glsl");

    // Material constructor requires these attributes
    glm::vec3 ambient(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse(0.5f, 0.5f, 0.5f);
    glm::vec3 specular(1.0f, 1.0f, 1.0f);
    float shininess = 32.0f;

    Material material(shader, ambient, diffuse, specular, shininess);
    Renderer renderer;

    Camera camera(640.0f / 480.0f);

    Hypergraph hypergraph(10);
    std::cout << "Before adding hyperedges example:" << std::endl;
    addHyperedgesExample(hypergraph);
    std::cout << "After adding hyperedges example:" << std::endl;
    std::cout << "Before adding hyperedges (manually):" << std::endl;
    hypergraph.addHyperedge({0, 1, 2});
    hypergraph.addHyperedge({2, 3, 4});
    hypergraph.addHyperedge({4, 5, 6});
    std::cout << "After adding hyperedges (manually):" << std::endl;
    hypergraph.printHypergraph();
    std::cout << "Before applying the rule (incl. state):" << std::endl;
    applyRule(hypergraph);
    // Update the node and edge mesh data with the hypergraph's data
    std::vector<Vertex> nodeVertices;
    std::vector<GLuint> nodeIndices;
    std::vector<Vertex> lineVertices;
    std::vector<GLuint> lineIndices;

    generateHypergraphRenderingData(hypergraph, nodeVertices, nodeIndices, lineVertices, lineIndices);

    // Create updated meshes
    Mesh nodeMesh(nodeVertices, nodeIndices);
    Mesh edgeMesh(lineVertices, lineIndices);
    LineData lineData = hypergraph.getLineData();
    std::cout << "After applying the rule:" << std::endl;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        // Render the scene
        renderer.render(nodeMesh, material, camera);
        renderer.render(edgeMesh, material, camera);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }
}