// app.cpp (or icp.cpp)
#include "app.hpp"
#include "../ShaderProgram.hpp"
#include "../OBJloader.hpp" // Required for loadOBJ

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <algorithm> // For std::max and std::min
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>
#include <io.h>
#include "../OBJloader.hpp"

using json = nlohmann::json;

// --- Helper for Task 2: Multi-monitor setup ---
GLFWmonitor* App::getCurrentMonitor(GLFWwindow* window) {
    int nmonitors, i;
    int wx, wy, ww, wh;
    int mx, my, mw, mh;
    int overlap, bestoverlap;
    GLFWmonitor* bestmonitor;
    GLFWmonitor** monitors;
    const GLFWvidmode* mode;

    bestoverlap = 0;
    bestmonitor = nullptr;

    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++) {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        // Calculate intersection area between window and monitor
        int overlapArea = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) * std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

        if (bestoverlap < overlapArea) {
            bestoverlap = overlapArea;
            bestmonitor = monitors[i];
        }
    }
    return bestmonitor ? bestmonitor : glfwGetPrimaryMonitor();
}

// --- Static Callback Implementations ---
void App::glfw_error_callback(int error, const char* description) {
    throw std::runtime_error("GLFW Error (" + std::to_string(error) + "): " + std::string(description));
}

void APIENTRY App::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cerr << "GL Debug (" << id << "): " << message << std::endl;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // Quit handling
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle VSync
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        app->vsyncEnabled = !app->vsyncEnabled;
        glfwSwapInterval(app->vsyncEnabled ? 1 : 0);
    }

    // Task 1.2: Capture/Release cursor
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        app->isCursorCaptured = !app->isCursorCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, app->isCursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    // Task 2: Toggle Full-screen mode
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        app->isFullscreen = !app->isFullscreen;
        if (app->isFullscreen) {
            // Save current position and size
            glfwGetWindowPos(window, &app->prevWinPos[0], &app->prevWinPos[1]);
            glfwGetWindowSize(window, &app->prevWinSize[0], &app->prevWinSize[1]);

            // Get mostly overlapped monitor
            GLFWmonitor* monitor = app->getCurrentMonitor(window);
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            // Restore position and size
            glfwSetWindowMonitor(window, nullptr, app->prevWinPos[0], app->prevWinPos[1], app->prevWinSize[0], app->prevWinSize[1], 0);
        }
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // ImGui needs to know about mouse interactions
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return; // Prevent clicking through ImGui windows

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        app->bgColor[0] = (rand() % 100) / 100.0f;
        app->bgColor[1] = (rand() % 100) / 100.0f;
        app->bgColor[2] = (rand() % 100) / 100.0f;
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->triangleColor[2] += (float)yoffset * 0.1f;
    if (app->triangleColor[2] > 1.0f) app->triangleColor[2] = 1.0f;
    if (app->triangleColor[2] < 0.0f) app->triangleColor[2] = 0.0f;
}

// --- Class Methods ---
App::App() { std::cout << "Constructed...\n"; }

bool App::init() {
    try {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

        std::ifstream f("config.json");
        if (f.is_open()) {
            json data = json::parse(f);
            winWidth = data.value("width", 800);
            winHeight = data.value("height", 600);
        }
        else {
            std::cerr << "config.json not found. Using default resolution.\n";
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        // Task 1.3: Hide window during startup
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(winWidth, winHeight, "OpenGL Lab", nullptr, nullptr);
        if (!window) throw std::runtime_error("Failed to create GLFW window");

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW");
        glGetError();

        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, fbsize_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);

        int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        glfwSwapInterval(vsyncEnabled ? 1 : 0);

        // Task 1.1: Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        // ==========================================
        // MODULAR SHADER COMPILATION & GEOMETRY SETUP
        // ==========================================

        // 1. Initialize Shader
        shader = std::make_unique<ShaderProgram>(std::filesystem::path("basic.vert"), std::filesystem::path("basic.frag"));

        // 2. Load the OBJ file into vectors FIRST
        std::vector<Vertex> loaded_vertices;
        std::vector<GLuint> loaded_indices;

        if (loadOBJ("triangle.obj", loaded_vertices, loaded_indices)) {
            // 3. Pass the loaded vectors to the Mesh constructor
            myModel = std::make_unique<Mesh>(loaded_vertices, loaded_indices, GL_TRIANGLES);
        }
        else {
            // 4. Throw error if file is missing so we don't crash with a nullptr later!
            throw std::runtime_error("CRITICAL ERROR: Could not find or load triangle.obj! Check your file path.");
        }

        // Task 1.3: Show window after everything is loaded
        glfwShowWindow(window);

    }
    catch (std::exception const& e) {
        std::cerr << "Init failed : " << e.what() << std::endl;
        throw;
    }
    std::cout << "Initialized...\n";
    return true;
}

int App::run(void) {
    try {
        double previousTime = glfwGetTime();
        int frameCount = 0;

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            frameCount++;
            if (currentTime - previousTime >= 1.0) {
                fps = frameCount;
                frameCount = 0;
                previousTime = currentTime;
                std::string title = "FPS: " + std::to_string(fps) + " | VSync: " + (vsyncEnabled ? "ON" : "OFF");
                glfwSetWindowTitle(window, title.c_str());
            }

            // --- Task 1.1: Start ImGui Frame ---
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Create a simple ImGui debug window
            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS: %d", fps);
            ImGui::Text("VSync: %s", vsyncEnabled ? "ON" : "OFF");
            ImGui::Text("Cursor Captured: %s (Press TAB)", isCursorCaptured ? "YES" : "NO");
            ImGui::ColorEdit3("Background Color", bgColor);
            ImGui::End();

            // Clear canvas
            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ==========================================
            // RENDER SCENE USING MODULAR CLASSES
            // ==========================================
            float timeValue = glfwGetTime();
            triangleColor[1] = (sin(timeValue) / 2.0f) + 0.5f;

            // 1. Bind the shader
            shader->use();

            // 2. Pass uniforms (Packed into a glm::vec4)
            shader->setUniform("ourColor", glm::vec4(triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]));

            // 3. Draw the model
            myModel->draw();

            // --- Task 1.1: Render ImGui over the scene ---
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Finished OK...\n";
    return EXIT_SUCCESS;
}

App::~App() {
    // Task 1.1: Cleanup ImGui resources
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Notice: OpenGL buffers and shaders are now cleaned up automatically
    // when the App is destroyed, thanks to std::unique_ptr and the RAII pattern!

    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}