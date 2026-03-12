#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "Shader.h"
#include "Shaders_embedded.h"
#include "Camera.h"
#include "Hypergraph.h"
#include "ForceLayout.h"
#include "RuleEngine.h"
#include "GraphIO.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Color modes
enum ColorMode { COLOR_DEGREE = 0, COLOR_AGE = 1 };
static const char* colorModeNames[] = { "Degree", "Age" };

// Pre-allocated rendering buffers — cleared and refilled each frame, never freed
static std::vector<Vertex> nodeVerts;
static std::vector<GLuint> nodeIdx;
static std::vector<Vertex> lineVerts;
static std::vector<GLuint> lineIdx;

// Vertex color cache (reused each frame)
static std::vector<float> vertColors; // packed r,g,b per vertex

static void computeVertexColors(const Hypergraph& graph, int colorMode) {
    const size_t n = graph.getNumVertices();
    vertColors.resize(n * 3);

    if (colorMode == COLOR_DEGREE) {
        for (size_t i = 0; i < n; ++i) {
            size_t deg = graph.getDegree(static_cast<uint32_t>(i));
            float t;
            if (deg <= 2) t = 0.0f;
            else if (deg <= 5) t = static_cast<float>(deg - 2) / 3.0f;
            else t = 1.0f;
            // dim blue -> cyan -> bright white-cyan
            float r = 0.2f + t * 0.5f;
            float g = 0.4f + t * 0.6f;
            float b = 0.8f + t * 0.2f;
            vertColors[3 * i]     = r;
            vertColors[3 * i + 1] = g;
            vertColors[3 * i + 2] = b;
        }
    } else { // COLOR_AGE
        float invN = (n > 1) ? 1.0f / static_cast<float>(n - 1) : 0.0f;
        for (size_t i = 0; i < n; ++i) {
            float t = static_cast<float>(i) * invN; // 0 = oldest, 1 = newest
            // warm orange -> cool blue
            float r = 1.0f - t * 0.9f;
            float g = 0.5f;
            float b = 0.1f + t * 0.9f;
            vertColors[3 * i]     = r;
            vertColors[3 * i + 1] = g;
            vertColors[3 * i + 2] = b;
        }
    }
}

static void generateRenderingData(const Hypergraph& graph) {
    const size_t n = graph.getNumVertices();
    const size_t ne = graph.getNumEdges();
    const glm::vec3* pos = graph.positionData();

    nodeVerts.resize(n);
    nodeIdx.resize(n);
    for (size_t i = 0; i < n; ++i) {
        float r = vertColors[3 * i];
        float g = vertColors[3 * i + 1];
        float b = vertColors[3 * i + 2];
        nodeVerts[i] = {{pos[i].x, pos[i].y, pos[i].z}, {r, g, b}};
        nodeIdx[i] = static_cast<GLuint>(i);
    }

    lineVerts.resize(ne * 2);
    lineIdx.resize(ne * 2);
    size_t li = 0;
    for (size_t i = 0; i < ne; ++i) {
        uint32_t a = graph.edgeA(i), b = graph.edgeB(i);
        if (a == b) continue;
        const glm::vec3& p1 = pos[a];
        const glm::vec3& p2 = pos[b];
        // Edge color = average of endpoint colors
        float r1 = vertColors[3 * a], g1 = vertColors[3 * a + 1], b1 = vertColors[3 * a + 2];
        float r2 = vertColors[3 * b], g2 = vertColors[3 * b + 1], b2 = vertColors[3 * b + 2];
        float mr = (r1 + r2) * 0.5f, mg = (g1 + g2) * 0.5f, mb = (b1 + b2) * 0.5f;
        lineVerts[li] = {{p1.x, p1.y, p1.z}, {mr, mg, mb}};
        lineIdx[li] = static_cast<GLuint>(li);
        li++;
        lineVerts[li] = {{p2.x, p2.y, p2.z}, {mr, mg, mb}};
        lineIdx[li] = static_cast<GLuint>(li);
        li++;
    }
    lineVerts.resize(li);
    lineIdx.resize(li);
}

static std::string takeScreenshot(GLFWwindow* window) {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    if (w <= 0 || h <= 0) return "Screenshot failed!";

    std::vector<unsigned char> pixels(w * h * 3);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Flip vertically (OpenGL is bottom-up)
    std::vector<unsigned char> flipped(w * h * 3);
    for (int row = 0; row < h; ++row) {
        std::memcpy(&flipped[row * w * 3], &pixels[(h - 1 - row) * w * 3], w * 3);
    }

    std::filesystem::create_directories("screenshots");
    std::time_t now = std::time(nullptr);
    char buf[128];
    std::strftime(buf, sizeof(buf), "screenshots/screenshot_%Y%m%d_%H%M%S.png",
                  std::localtime(&now));

    if (stbi_write_png(buf, w, h, 3, flipped.data(), w * 3)) {
        return std::string("Saved: ") + buf;
    }
    return "Screenshot failed!";
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Hypergraph 3D", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.0f;
        style.FrameRounding = 4.0f;
        style.GrabRounding = 3.0f;
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 6);
        style.WindowBorderSize = 0.0f;
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.10f, 0.90f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.26f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.7f, 0.9f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.5f, 0.7f, 0.6f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.6f, 0.8f, 0.8f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.7f, 0.9f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.5f, 0.7f, 0.4f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.6f, 0.8f, 0.6f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.08f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.3f, 0.5f, 1.0f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.22f, 1.0f);
    }

    Shader shader = Shader::fromSource(EmbeddedShaders::vertexShader, EmbeddedShaders::fragmentShader);
    Material material(shader, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(1.0f), 32.0f);
    Renderer renderer;
    Camera camera(1024.0f / 768.0f);

    RuleEngine ruleEngine;
    Hypergraph graph(0);
    ruleEngine.seedGraph(graph, ruleEngine.getCurrentRule());

    ForceLayout layout;
    size_t lastCoarsenSize = 0;
    Mesh nodeMesh(GL_POINTS);
    Mesh edgeMesh(GL_LINES);

    // Pre-reserve rendering buffers
    nodeVerts.reserve(10000);
    nodeIdx.reserve(10000);
    lineVerts.reserve(40000);
    lineIdx.reserve(40000);

    int frameCount = 0;
    int currentRuleUI = static_cast<int>(ruleEngine.getCurrentRule());
    bool running = false;
    int ruleInterval = 1;
    int maxEdges = 20000;
    int colorMode = COLOR_DEGREE;
    std::string statusMessage;

    bool dragging = false;
    double prevMouseX = 0.0, prevMouseY = 0.0;

    // FPS tracking
    double fpsLastTime = glfwGetTime();
    int fpsFrameCount = 0;
    float displayFps = 0.0f;

    // Cache dropdown labels (avoid string concat every frame)
    std::vector<std::string> ruleLabels;
    for (size_t i = 0; i < ruleEngine.getNumRules(); ++i) {
        ruleLabels.push_back(ruleEngine.getRuleName(i) + " — " + ruleEngine.getRuleDescription(i));
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // FPS calculation
        fpsFrameCount++;
        double fpsNow = glfwGetTime();
        if (fpsNow - fpsLastTime >= 0.5) {
            displayFps = static_cast<float>(fpsFrameCount) / static_cast<float>(fpsNow - fpsLastTime);
            fpsFrameCount = 0;
            fpsLastTime = fpsNow;
        }

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        if (fbW > 0 && fbH > 0) {
            camera.setAspectRatio(static_cast<float>(fbW) / static_cast<float>(fbH));
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Mouse orbit
        if (!ImGui::GetIO().WantCaptureMouse) {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                if (dragging) {
                    camera.rotate(static_cast<float>(mx - prevMouseX) * 0.005f,
                                  static_cast<float>(my - prevMouseY) * 0.005f);
                }
                dragging = true;
                prevMouseX = mx;
                prevMouseY = my;
            } else {
                dragging = false;
            }

            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                camera.zoom(-wheel * 0.5f);
            }
        } else {
            dragging = false;
        }

        // ImGui panel
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(340, 0), ImVec2(520, FLT_MAX));
        ImGui::SetNextWindowSize(ImVec2(360, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Hypergraph 3D");
        ImGui::BeginTabBar("MainTabs");
        if (ImGui::BeginTabItem("Controls")) {

        // Start / Stop / Reset buttons
        bool atLimit = static_cast<int>(graph.getNumEdges()) >= maxEdges;
        if (!running) {
            if (ImGui::Button("Start", ImVec2(95, 36))) {
                running = true;
                if (graph.getNumVertices() == 0) {
                    ruleEngine.seedGraph(graph, ruleEngine.getCurrentRule());
                    lastCoarsenSize = 0;
                    frameCount = 0;
                }
            }
        } else {
            if (ImGui::Button("Stop", ImVec2(95, 36))) {
                running = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset", ImVec2(95, 36))) {
            running = false;
            graph = Hypergraph(0);
            ruleEngine.seedGraph(graph, ruleEngine.getCurrentRule());
            lastCoarsenSize = 0;
            frameCount = 0;
        }

        ImGui::SameLine();
        if (running) {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.4f, 1.0f), atLimit ? "LIMIT" : "RUNNING");
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "STOPPED");
        }

        ImGui::Separator();

        // Max edges — editable only when stopped
        if (running) ImGui::BeginDisabled();
        ImGui::InputInt("Max Edges", &maxEdges, 1000, 5000);
        if (maxEdges < 100) maxEdges = 100;
        if (maxEdges > 200000) maxEdges = 200000;
        if (running) ImGui::EndDisabled();

        ImGui::SliderInt("Interval", &ruleInterval, 1, 30);

        ImGui::Separator();
        ImGui::Text("Vertices: %zu", graph.getNumVertices());
        ImGui::Text("Edges: %zu / %d", graph.getNumEdges(), maxEdges);
        ImGui::Text("FPS: %.0f", displayFps);

        ImGui::Separator();
        ImGui::SliderFloat("Zoom", &camera.distance, 0.5f, 100.0f, "%.1f");
        ImGui::SliderFloat("Spread", &layout.restLength, 0.05f, 1.0f, "%.2f");

        // Color mode selector
        ImGui::Combo("Color", &colorMode, colorModeNames, IM_ARRAYSIZE(colorModeNames));

        ImGui::Separator();

        // Rule selector — disabled while running
        if (running) ImGui::BeginDisabled();
        if (ImGui::BeginCombo("Rule", ruleEngine.getRuleName(currentRuleUI).c_str())) {
            for (size_t i = 0; i < ruleEngine.getNumRules(); ++i) {
                bool selected = (currentRuleUI == static_cast<int>(i));
                if (ImGui::Selectable(ruleLabels[i].c_str(), selected)) {
                    currentRuleUI = static_cast<int>(i);
                    ruleEngine.setCurrentRule(i);
                    graph = Hypergraph(0);
                    ruleEngine.seedGraph(graph, i);
                    lastCoarsenSize = 0;
                    frameCount = 0;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (running) ImGui::EndDisabled();

        ImGui::Separator();

        if (ImGui::Button("Save")) {
            std::string path = GraphIO::generateFilepath();
            if (GraphIO::save(graph, ruleEngine.getCurrentRule(), path)) {
                statusMessage = "Saved: " + path;
            } else {
                statusMessage = "Save failed!";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Screenshot")) {
            statusMessage = takeScreenshot(window);
        }

        static char loadPath[256] = "saves/";
        ImGui::InputText("##loadpath", loadPath, sizeof(loadPath));
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            size_t loadedRule = 0;
            if (GraphIO::load(graph, loadedRule, std::string(loadPath))) {
                ruleEngine.setCurrentRule(loadedRule);
                currentRuleUI = static_cast<int>(loadedRule);
                statusMessage = "Loaded: " + std::string(loadPath);
                layout.computeInitialLayout(graph);
                lastCoarsenSize = graph.getNumVertices();
                frameCount = 0;
            } else {
                statusMessage = "Load failed!";
            }
        }

        if (!statusMessage.empty()) {
            ImGui::TextWrapped("%s", statusMessage.c_str());
        }

        ImGui::EndTabItem();
        } // end Controls tab

        if (ImGui::BeginTabItem("About")) {
            ImGui::Spacing();
            ImGui::TextWrapped(
                "Real-time 3D visualization of Wolfram Physics-style "
                "hypergraph rewriting.");
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "How it works");
            ImGui::TextWrapped(
                "The simulation starts from a single vertex with two self-loops. "
                "Each step, the engine finds two edges that share a vertex, removes "
                "them, creates a new vertex, and wires in replacement edges according "
                "to the selected rule. Different rules produce very different "
                "structures.");
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Rules");
            ImGui::BulletText("Branching: dense, interconnected clusters");
            ImGui::BulletText("Looping: long chains and ring structures");
            ImGui::BulletText("Spreading: radial, star-like patterns");
            ImGui::BulletText("Knitting: branching tree structures");
            ImGui::BulletText("Chain: long filaments with cross-links");
            ImGui::BulletText("Crystalline: dense lattice growth");
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Layout");
            ImGui::TextWrapped(
                "The layout uses two techniques: multilevel coarsening (Walshaw-style) "
                "sets the macro structure periodically, and local-only forces run every "
                "frame. Repulsion only acts between directly connected vertices, which "
                "prevents the graph from collapsing into a sphere.");
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "Controls");
            ImGui::BulletText("Click and drag to orbit");
            ImGui::BulletText("Scroll to zoom");
            ImGui::BulletText("Start/Stop to run the simulation");
            ImGui::BulletText("Reset to start over");
            ImGui::Spacing();

            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "Hypergraph 3D v1.2.0");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                "github.com/alexfacehead/directed-graph-3D");

            ImGui::EndTabItem();
        } // end About tab

        ImGui::EndTabBar();
        ImGui::End();

        if (running) {
            if (frameCount % ruleInterval == 0
                && static_cast<int>(graph.getNumEdges()) < maxEdges) {
                ruleEngine.applyStep(graph);
            }

            size_t nv = graph.getNumVertices();
            size_t coarsenCap = static_cast<size_t>(maxEdges) / 4;
            if (coarsenCap < 2000) coarsenCap = 2000;
            if (nv >= 30 && nv <= coarsenCap && (lastCoarsenSize == 0 || nv >= lastCoarsenSize * 3 / 2)) {
                layout.computeInitialLayout(graph, nv);
                lastCoarsenSize = nv;
            }

            layout.step(graph);
        }

        // Compute colors and generate rendering data
        computeVertexColors(graph, colorMode);
        generateRenderingData(graph);
        nodeMesh.update(nodeVerts, nodeIdx);
        edgeMesh.update(lineVerts, lineIdx);

        // Camera target tracks graph center
        size_t n = graph.getNumVertices();
        if (n > 0) {
            const glm::vec3* pos = graph.positionData();
            glm::vec3 bmin = pos[0], bmax = pos[0];
            for (size_t i = 1; i < n; ++i) {
                bmin = glm::min(bmin, pos[i]);
                bmax = glm::max(bmax, pos[i]);
            }
            camera.target = (bmin + bmax) * 0.5f;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.render(edgeMesh, material, camera);
        renderer.render(nodeMesh, material, camera);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        frameCount++;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
