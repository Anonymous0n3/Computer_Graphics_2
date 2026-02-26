// icp.cpp // author: JJ
#include "app.hpp"
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
        // SHADER COMPILATION & GEOMETRY SETUP
        // ==========================================
        const char* vertexShaderSource = "#version 460 core\n"
            "layout (location = 0) in vec3 aPos;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
            "}\0";

        const char* fragmentShaderSource = "#version 460 core\n"
            "out vec4 FragColor;\n"
            "uniform vec4 ourColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = ourColor;\n"
            "}\n\0";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // left  
             0.5f, -0.5f, 0.0f, // right 
             0.0f,  0.5f, 0.0f  // top   
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        uniform_color_location = glGetUniformLocation(shaderProgram, "ourColor");

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
            ImGui::ColorEdit3("Background Color", bgColor); // Bonus: edit background color interactively!
            ImGui::End();

            // Clear canvas
            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render Triangle
            glUseProgram(shaderProgram);
            float timeValue = glfwGetTime();
            triangleColor[1] = (sin(timeValue) / 2.0f) + 0.5f;

            if (uniform_color_location != -1) {
                glUniform4f(uniform_color_location, triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]);
            }

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

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

    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);

    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}