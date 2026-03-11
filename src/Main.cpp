#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Mesh.h"
#include "Material.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "Hypergraph.h"
#include "ForceLayout.h"
#include "RuleEngine.h"
#include "GraphIO.h"

static const float CYAN_R = 0.0f;
static const float CYAN_G = 0.8f;
static const float CYAN_B = 1.0f;

// Pre-allocated rendering buffers — cleared and refilled each frame, never freed
static std::vector<Vertex> nodeVerts;
static std::vector<GLuint> nodeIdx;
static std::vector<Vertex> lineVerts;
static std::vector<GLuint> lineIdx;

static void generateRenderingData(const Hypergraph& graph) {
    const size_t n = graph.getNumVertices();
    const size_t ne = graph.getNumEdges();
    const glm::vec3* pos = graph.positionData();

    nodeVerts.resize(n);
    nodeIdx.resize(n);
    for (size_t i = 0; i < n; ++i) {
        nodeVerts[i] = {{pos[i].x, pos[i].y, pos[i].z}, {CYAN_R, CYAN_G, CYAN_B}};
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
        lineVerts[li] = {{p1.x, p1.y, p1.z}, {CYAN_R, CYAN_G, CYAN_B}};
        lineIdx[li] = static_cast<GLuint>(li);
        li++;
        lineVerts[li] = {{p2.x, p2.y, p2.z}, {CYAN_R, CYAN_G, CYAN_B}};
        lineIdx[li] = static_cast<GLuint>(li);
        li++;
    }
    lineVerts.resize(li);
    lineIdx.resize(li);
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

    glewExperimental = GL_TRUE;
    glewInit();

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

    Shader shader("./vertex_shader.glsl", "./fragment_shader.glsl");
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

    // Pre-reserve rendering buffers for max capacity
    const size_t maxVertices = 10000;
    nodeVerts.reserve(maxVertices);
    nodeIdx.reserve(maxVertices);
    lineVerts.reserve(maxVertices * 4);
    lineIdx.reserve(maxVertices * 4);

    int frameCount = 0;
    int currentRuleUI = static_cast<int>(ruleEngine.getCurrentRule());
    bool paused = false;
    int ruleInterval = 1;
    std::string statusMessage;

    bool dragging = false;
    double prevMouseX = 0.0, prevMouseY = 0.0;

    // Cache dropdown labels (avoid string concat every frame)
    std::vector<std::string> ruleLabels;
    for (size_t i = 0; i < ruleEngine.getNumRules(); ++i) {
        ruleLabels.push_back(ruleEngine.getRuleName(i) + " — " + ruleEngine.getRuleDescription(i));
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

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
        ImGui::SetNextWindowSize(ImVec2(280, 0), ImGuiCond_FirstUseEver);
        ImGui::Begin("Controls");

        ImGui::Checkbox("Pause", &paused);
        ImGui::SliderInt("Interval", &ruleInterval, 1, 30);

        ImGui::Text("Vertices: %zu", graph.getNumVertices());
        ImGui::Text("Edges: %zu", graph.getNumEdges());

        ImGui::Separator();
        ImGui::SliderFloat("Zoom", &camera.distance, 0.5f, 100.0f, "%.1f");
        ImGui::Separator();

        if (ImGui::BeginCombo("Rule", ruleEngine.getRuleName(currentRuleUI).c_str())) {
            for (size_t i = 0; i < ruleEngine.getNumRules(); ++i) {
                bool selected = (currentRuleUI == static_cast<int>(i));
                if (ImGui::Selectable(ruleLabels[i].c_str(), selected)) {
                    currentRuleUI = static_cast<int>(i);
                    ruleEngine.setCurrentRule(i);
                    ruleEngine.seedGraph(graph, i);
                    lastCoarsenSize = 0;
                    frameCount = 0;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

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

        ImGui::End();

        if (!paused) {
            if (frameCount % ruleInterval == 0
                && graph.getNumVertices() < maxVertices) {
                ruleEngine.applyStep(graph);
            }

            size_t nv = graph.getNumVertices();
            if (nv >= 30 && nv <= 2000 && (lastCoarsenSize == 0 || nv >= lastCoarsenSize * 2)) {
                layout.computeInitialLayout(graph);
                lastCoarsenSize = nv;
            }

            layout.step(graph);
        }

        // Generate rendering data (uses pre-reserved buffers)
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
