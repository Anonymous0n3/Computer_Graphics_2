// app.cpp
#include "app.hpp"
#include "../ShaderProgram.hpp"
#include "../OBJloader.hpp" 
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <algorithm> 
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

using json = nlohmann::json;

void App::update_projection_matrix() {
    if (winHeight < 1) winHeight = 1;
    float ratio = static_cast<float>(winWidth) / winHeight;

    projection_matrix = glm::perspective(
        glm::radians(fov),
        ratio,
        0.1f,
        20000.0f
    );
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->winWidth = width;
    app->winHeight = height;
    glViewport(0, 0, width, height);
    app->update_projection_matrix();
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->fov -= (float)yoffset * 5.0f;
    app->fov = std::clamp(app->fov, 20.0f, 170.0f);
    app->update_projection_matrix();
}

GLFWmonitor* App::getCurrentMonitor(GLFWwindow* window) {
    int nmonitors, i;
    int wx, wy, ww, wh;
    int mx, my, mw, mh;
    int overlap = 0, bestoverlap = 0;
    GLFWmonitor* bestmonitor = nullptr;
    GLFWmonitor** monitors;
    const GLFWvidmode* mode;

    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++) {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        int overlapArea = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) * std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

        if (bestoverlap < overlapArea) {
            bestoverlap = overlapArea;
            bestmonitor = monitors[i];
        }
    }
    return bestmonitor ? bestmonitor : glfwGetPrimaryMonitor();
}

void App::glfw_error_callback(int error, const char* description) {
    throw std::runtime_error("GLFW Error (" + std::to_string(error) + "): " + std::string(description));
}

void APIENTRY App::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cerr << "GL Debug (" << id << "): " << message << std::endl;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        app->vsyncEnabled = !app->vsyncEnabled;
        glfwSwapInterval(app->vsyncEnabled ? 1 : 0);
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        app->isCursorCaptured = !app->isCursorCaptured;
        glfwSetInputMode(window, GLFW_CURSOR, app->isCursorCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
        app->isFullscreen = !app->isFullscreen;
        if (app->isFullscreen) {
            glfwGetWindowPos(window, &app->prevWinPos[0], &app->prevWinPos[1]);
            glfwGetWindowSize(window, &app->prevWinSize[0], &app->prevWinSize[1]);
            GLFWmonitor* monitor = app->getCurrentMonitor(window);
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else {
            glfwSetWindowMonitor(window, nullptr, app->prevWinPos[0], app->prevWinPos[1], app->prevWinSize[0], app->prevWinSize[1], 0);
        }
    }
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        app->bgColor[0] = (rand() % 100) / 100.0f;
        app->bgColor[1] = (rand() % 100) / 100.0f;
        app->bgColor[2] = (rand() % 100) / 100.0f;
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (!app->isCursorCaptured) return;

    if (app->firstMouse) {
        app->cursorLastX = xpos;
        app->cursorLastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - app->cursorLastX);
    float yoffset = static_cast<float>(app->cursorLastY - ypos);

    app->cursorLastX = xpos;
    app->cursorLastY = ypos;

    app->myCam.ProcessMouseMovement(xoffset, yoffset);
}

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
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(winWidth, winHeight, "OpenGL Lab", nullptr, nullptr);
        if (!window) throw std::runtime_error("Failed to create GLFW window");

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW");
        glGetError();

        // FIX: Enable Z-Buffer so 3D objects don't overwrite each other weirdly!
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // Draw both sides of the triangle!

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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        shader = std::make_shared<ShaderProgram>(std::filesystem::path("basic.vert"), std::filesystem::path("basic.frag"));

        std::vector<Vertex> loaded_vertices;
        std::vector<GLuint> loaded_indices;

        if (loadOBJ("triangle.obj", loaded_vertices, loaded_indices)) {
            auto rawMesh = std::make_shared<Mesh>(loaded_vertices, loaded_indices, GL_TRIANGLES);
            mySceneObject = std::make_unique<Model>();
            mySceneObject->addMesh(rawMesh, shader);
        }

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
        double fpsTimer = glfwGetTime();
        lastFrameTime = glfwGetTime(); // FIX: Initialize the camera delta timer
        int frameCount = 0;

        // FIX: Setup initial camera lens before entering the loop
        update_projection_matrix();

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();

            // --- FPS CALCULATION ---
            frameCount++;
            if (currentTime - fpsTimer >= 1.0) {
                fps = frameCount;
                frameCount = 0;
                fpsTimer = currentTime;
                std::string title = "FPS: " + std::to_string(fps) + " | VSync: " + (vsyncEnabled ? "ON" : "OFF");
                glfwSetWindowTitle(window, title.c_str());
            }

            // --- CAMERA DELTA TIME ---
            // FIX: Correctly calculate the fraction of a second since the last frame
            float deltaTime = static_cast<float>(currentTime - lastFrameTime);
            lastFrameTime = currentTime;

            // --- PROCESS INPUT ---
            myCam.Position += myCam.ProcessInput(window, deltaTime);

            // --- CLEAR SCREEN ---
            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // --- IMGUI SETUP ---
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS: %d", fps);
            ImGui::Text("VSync: %s", vsyncEnabled ? "ON" : "OFF");
            ImGui::Text("Cursor Captured: %s (Press TAB)", isCursorCaptured ? "YES" : "NO");
            ImGui::ColorEdit3("Background Color", bgColor);
            ImGui::End();

            // --- RENDER SCENE ---
            if (mySceneObject) { // Ensure it loaded successfully
                shader->use();

                // Pass matrices
                shader->setUniform("uP_m", projection_matrix);
                shader->setUniform("uV_m", myCam.GetViewMatrix());
                shader->setUniform("ourColor", glm::vec4(triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]));
                // Spin the object using global time
                //mySceneObject->setEulerAngles(glm::vec3(0.0f, static_cast<float>(currentTime) * 45.0f, 0.0f));
                // Try spinning it on the Z axis instead to see if it becomes visible
                mySceneObject->setEulerAngles(glm::vec3(0.0f, 0.0f, static_cast<float>(currentTime) * 45.0f));
                // Draw it!
                mySceneObject->draw();
            }

            // --- IMGUI RENDER ---
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // --- SWAP BUFFERS AND POLL ---
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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}