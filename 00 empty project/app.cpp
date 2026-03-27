#include "app.hpp"
#include "../ShaderProgram.hpp"
#include "../OBJloader.hpp"
#include "../Texture.hpp" // Přidáno pro Task 4

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
#include "../Mesh.hpp"

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

    // Task 2 (from previous): Toggle Full-screen mode
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

    // --- Task 1: Toggle MSAA ---
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        app->msaaEnabled = !app->msaaEnabled;
        if (app->msaaEnabled) {
            glEnable(GL_MULTISAMPLE);
            std::cout << "MSAA Zapnuto\n";
        }
        else {
            glDisable(GL_MULTISAMPLE);
            std::cout << "MSAA Vypnuto\n";
        }
    }

    // --- Task 2: Screenshot ---
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        cv::Mat img(app->winHeight, app->winWidth, CV_8UC3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, app->winWidth, app->winHeight, GL_BGR, GL_UNSIGNED_BYTE, img.data);
        cv::flip(img, img, 0); // Otevřené okno má 0,0 vlevo dole, OpenCV vlevo nahoře

        std::string filename = app->msaaEnabled ? "screenshot_msaa_on.png" : "screenshot_msaa_off.png";
        cv::imwrite(filename, img);
        std::cout << "Screenshot uložen jako: " << filename << "\n";
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (height == 0) height = 1; // Prevent divide by zero
    app->projection = glm::perspective(glm::radians(app->fov), (float)width / (float)height, 0.1f, 100.0f);
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
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    float xoffset = xpos - app->lastX;
    float yoffset = app->lastY - ypos;
    app->lastX = xpos;
    app->lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->yaw += xoffset;
    app->pitch += yoffset;

    if (app->pitch > 89.0f)  app->pitch = 89.0f;
    if (app->pitch < -89.0f) app->pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    front.y = sin(glm::radians(app->pitch));
    front.z = sin(glm::radians(app->yaw)) * cos(glm::radians(app->pitch));
    app->cameraFront = glm::normalize(front);
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

        // --- Task 1: MSAA 4x Hint ---
        glfwWindowHint(GLFW_SAMPLES, 4);

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

        // --- Task 1: Enable MSAA by default ---
        if (msaaEnabled) {
            glEnable(GL_MULTISAMPLE);
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        // ==========================================
        // MODULAR SHADER COMPILATION & GEOMETRY SETUP
        // ==========================================

        shader = std::make_unique<ShaderProgram>(std::filesystem::path("basic.vert"), std::filesystem::path("basic.frag"));

        std::vector<Vertex> loaded_vertices;
        std::vector<GLuint> loaded_indices;

        if (loadOBJ("bunny.obj", loaded_vertices, loaded_indices)) {
            myModel = std::make_unique<Mesh>(loaded_vertices, loaded_indices, GL_TRIANGLES);
        }
        else {
            throw std::runtime_error("CRITICAL ERROR: Could not find or load model! Check your file path.");
        }
        //myModel = std::make_unique<Mesh>(generateCube());
        // --- Task 4: Load Texture ---
        try {
            // Změň název souboru na reálný obrázek, který máš ve složce s projektem!
            myTexture = std::make_unique<Texture>(std::filesystem::path("box.jpg"));
            std::cout << "Textura uspesne nactena.\n";
        }
        catch (const std::exception& e) {
            std::cerr << "Varovani: Texturu se nepodarilo nacist. " << e.what() << "\n";
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
        double previousTime = glfwGetTime();
        int frameCount = 0;
        double lastFrameTime = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastFrameTime);
            lastFrameTime = currentTime;

            frameCount++;
            if (currentTime - previousTime >= 1.0) {
                fps = frameCount;
                frameCount = 0;
                previousTime = currentTime;
                std::string title = "FPS: " + std::to_string(fps) +
                    " | VSync: " + (vsyncEnabled ? "ON" : "OFF") +
                    " | MSAA: " + (msaaEnabled ? "ON" : "OFF"); // Přidáno MSAA do titulku
                glfwSetWindowTitle(window, title.c_str());
            }

            if (isCursorCaptured) {
                float cameraSpeed = 2.5f * deltaTime;
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    cameraPos += cameraSpeed * cameraFront;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    cameraPos -= cameraSpeed * cameraFront;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Debug Menu");
            ImGui::Text("FPS: %d", fps);
            ImGui::Text("VSync: %s (Press V)", vsyncEnabled ? "ON" : "OFF");
            ImGui::Text("MSAA: %s (Press M)", msaaEnabled ? "ON" : "OFF");
            ImGui::Text("Cursor Captured: %s (Press TAB)", isCursorCaptured ? "YES" : "NO");
            ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
            ImGui::ColorEdit3("Background Color", bgColor);
            if (ImGui::Button("Take Screenshot (Press P)")) {
                // Můžeš implementovat i tlačítko na screenshot
            }
            ImGui::End();

            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float timeValue = (float)glfwGetTime();
            // Upravil jsem barvu, ať je bílá a zbytečně netónuje texturu. Můžeš vrátit zpět na timeValue, pokud chceš.
            triangleColor[0] = 1.0f;
            triangleColor[1] = 1.0f;
            triangleColor[2] = 1.0f;

            shader->use();

            // --- Task 4: Bind Texture ---
            if (myTexture) {
                myTexture->bind(); // Připojí texturu k texturovací jednotce 0
                shader->setUniform("tex0", 0); // Řekne shaderu, aby četl z jednotky 0
            }

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::rotate(model, timeValue, glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            shader->setUniform("model", model);
            shader->setUniform("view", view);
            shader->setUniform("projection", projection);
            shader->setUniform("ourColor", glm::vec4(triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]));

            myModel->draw();

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
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}