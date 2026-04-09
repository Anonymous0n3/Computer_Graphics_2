#include "app.hpp"
#include "../ShaderProgram.hpp"
#include "../OBJloader.hpp"
#include "../Texture.hpp" 

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
#include "../Mesh.hpp"

using json = nlohmann::json;

// --- Multi-monitor helper ---
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

        int overlapArea = std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) * std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

        if (bestoverlap < overlapArea) {
            bestoverlap = overlapArea;
            bestmonitor = monitors[i];
        }
    }
    return bestmonitor ? bestmonitor : glfwGetPrimaryMonitor();
}

// --- Callbacks ---
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

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        cv::Mat img(app->winHeight, app->winWidth, CV_8UC3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, app->winWidth, app->winHeight, GL_BGR, GL_UNSIGNED_BYTE, img.data);
        cv::flip(img, img, 0);

        std::string filename = app->msaaEnabled ? "screenshot_msaa_on.png" : "screenshot_msaa_off.png";
        cv::imwrite(filename, img);
        std::cout << "Screenshot uložen jako: " << filename << "\n";
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (height == 0) height = 1;
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
        glfwWindowHint(GLFW_SAMPLES, 4); // MSAA
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

        window = glfwCreateWindow(winWidth, winHeight, "OpenGL Lab", nullptr, nullptr);
        if (!window) throw std::runtime_error("Failed to create GLFW window");

        glfwMakeContextCurrent(window);
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW");

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

        if (msaaEnabled) glEnable(GL_MULTISAMPLE);
        glEnable(GL_DEPTH_TEST);

        // --- Task 1: Enable Blending for transparency ---
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // --- Task 3: Enable Point Size for particles ---
        glEnable(GL_PROGRAM_POINT_SIZE);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460 core");

        shader = std::make_unique<ShaderProgram>(std::filesystem::path("basic.vert"), std::filesystem::path("basic.frag"));

        std::vector<Vertex> loaded_vertices;
        std::vector<GLuint> loaded_indices;

        if (loadOBJ("bunny.obj", loaded_vertices, loaded_indices)) {
            myModel = std::make_unique<Mesh>(loaded_vertices, loaded_indices, GL_TRIANGLES);

            // --- Task 2: Calculate local AABB for collision ---
            if (!loaded_vertices.empty()) {
                modelLocalAABB.min = loaded_vertices[0].Position;
                modelLocalAABB.max = loaded_vertices[0].Position;
                for (const auto& v : loaded_vertices) {
                    modelLocalAABB.min = glm::min(modelLocalAABB.min, v.Position);
                    modelLocalAABB.max = glm::max(modelLocalAABB.max, v.Position);
                }
                std::cout << "Model AABB vypocitan.\n";
            }
        }
        else {
            throw std::runtime_error("CRITICAL ERROR: Could not find or load model! Check your file path.");
        }

        try {
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

            float timeValue = (float)currentTime; // Vypocet casu pro rotace a animace

            frameCount++;
            if (currentTime - previousTime >= 1.0) {
                fps = frameCount;
                frameCount = 0;
                previousTime = currentTime;
                std::string title = "FPS: " + std::to_string(fps) +
                    " | VSync: " + (vsyncEnabled ? "ON" : "OFF") +
                    " | MSAA: " + (msaaEnabled ? "ON" : "OFF");
                glfwSetWindowTitle(window, title.c_str());
            }

            // ==========================================
            // INPUT A KOLIZE KAMERY
            // ==========================================
            if (isCursorCaptured) {
                float cameraSpeed = 2.5f * deltaTime;
                glm::vec3 nextPos = cameraPos;

                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) nextPos += cameraSpeed * cameraFront;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) nextPos -= cameraSpeed * cameraFront;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) nextPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) nextPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

                // --- Task 2: Puvodni kolize s hranicemi mapy ---
                if (nextPos.x < mapBounds.min.x || nextPos.x > mapBounds.max.x) nextPos.x = cameraPos.x;
                if (nextPos.z < mapBounds.min.z || nextPos.z > mapBounds.max.z) nextPos.z = cameraPos.z;

                // --- Task 2: Pokrocila kolize s libovolnymi modely ---
                bool hitModel = false;
                float playerRadius = 0.2f;

                std::vector<glm::mat4> sceneModels;

                // Pridani hlavniho rotujiciho modelu
                glm::mat4 mainModelMatrix = glm::mat4(1.0f);
                mainModelMatrix = glm::rotate(mainModelMatrix, timeValue * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                sceneModels.push_back(mainModelMatrix);

                // Pridani pruhlednych modelu do seznamu kolizi
                for (int i = 0; i < 3; i++) {
                    glm::mat4 transModel = glm::mat4(1.0f);
                    transModel = glm::translate(transModel, glm::vec3(3.0f, 0.0f, -2.0f - (i * 2.0f)));
                    sceneModels.push_back(transModel);
                }

                // Detekce kolize prevedenim do lokalniho prostoru modelu
                for (const auto& modelMatrix : sceneModels) {
                    glm::vec3 localNextPos = glm::vec3(glm::inverse(modelMatrix) * glm::vec4(nextPos, 1.0f));

                    if (localNextPos.x >= modelLocalAABB.min.x - playerRadius && localNextPos.x <= modelLocalAABB.max.x + playerRadius &&
                        localNextPos.y >= modelLocalAABB.min.y - playerRadius && localNextPos.y <= modelLocalAABB.max.y + playerRadius &&
                        localNextPos.z >= modelLocalAABB.min.z - playerRadius && localNextPos.z <= modelLocalAABB.max.z + playerRadius) {

                        hitModel = true;
                        break;
                    }
                }

                if (hitModel) {
                    nextPos = cameraPos;

                    // --- Task 3: Spawnovani jisker pri narazu ---
                    if (particles.size() < 50) {
                        for (int p = 0; p < 3; p++) {
                            particles.push_back({
                                cameraPos + cameraFront * 0.3f,
                                glm::vec3(((rand() % 100) / 50.f) - 1.f,
                                          ((rand() % 100) / 50.f) - 1.f,
                                          ((rand() % 100) / 50.f) - 1.f) * 2.0f,
                                0.3f + ((rand() % 100) / 200.f)
                                });
                        }
                    }
                }

                cameraPos = nextPos;
            }

            // ==========================================
            // IMGUI A CLEAR BUFFER
            // ==========================================
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
            ImGui::End();

            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->use();

            // Nastaveni materialu a ViewPos
            if (myTexture) {
                myTexture->bind();
                shader->setUniform("material.diffuse", 0);
            }
            shader->setUniform("material.shininess", 32.0f);
            shader->setUniform("viewPos", cameraPos);

            // Svetla
            glm::vec3 sunDir = glm::vec3(sin(timeValue), -1.0f, cos(timeValue));
            shader->setUniform("dirLight.direction", sunDir);
            shader->setUniform("dirLight.ambient", glm::vec3(0.05f));
            shader->setUniform("dirLight.diffuse", glm::vec3(0.4f));
            shader->setUniform("dirLight.specular", glm::vec3(0.5f));

            std::vector<glm::vec3> pointLightPositions = {
                glm::vec3(2.0f * sin(timeValue),  0.2f,  2.0f * cos(timeValue)),
                glm::vec3(2.3f, -1.3f, -2.0f),
                glm::vec3(-2.0f,  2.0f, -3.0f)
            };
            std::vector<glm::vec3> pointLightColors = {
                glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            };

            for (int i = 0; i < 3; i++) {
                std::string prefix = "pointLights[" + std::to_string(i) + "].";
                shader->setUniform(prefix + "position", pointLightPositions[i]);
                shader->setUniform(prefix + "ambient", pointLightColors[i] * 0.05f);
                shader->setUniform(prefix + "diffuse", pointLightColors[i] * 0.8f);
                shader->setUniform(prefix + "specular", pointLightColors[i] * 1.0f);
                shader->setUniform(prefix + "constant", 1.0f);
                shader->setUniform(prefix + "linear", 0.09f);
                shader->setUniform(prefix + "quadratic", 0.032f);
            }

            shader->setUniform("spotLight.position", cameraPos);
            shader->setUniform("spotLight.direction", cameraFront);
            shader->setUniform("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
            shader->setUniform("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
            shader->setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
            shader->setUniform("spotLight.constant", 1.0f);
            shader->setUniform("spotLight.linear", 0.09f);
            shader->setUniform("spotLight.quadratic", 0.032f);
            shader->setUniform("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            shader->setUniform("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
            shader->setUniform("view", view);
            shader->setUniform("projection", projection);


            // ==========================================
            // KRESLENI SCENY - SPRAVNE PORADI
            // ==========================================

            // 1. VYKRESLENÍ HLAVNÍCH NEPRŮHLEDNÝCH MODELŮ
            shader->setUniform("objectAlpha", 1.0f); // Plna viditelnost

            glm::mat4 mainModel = glm::mat4(1.0f);
            mainModel = glm::rotate(mainModel, timeValue * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
            shader->setUniform("model", mainModel);
            myModel->draw();


            // 2. UPDATE A VYKRESLENÍ ČÁSTIC (Task 3)
            for (auto& p : particles) {
                p.life -= deltaTime;
                p.position += p.velocity * deltaTime;
            }

            particles.erase(std::remove_if(particles.begin(), particles.end(),
                [](const Particle& p) { return p.life <= 0.0f; }), particles.end());

            if (!particles.empty()) {
                std::vector<Vertex> pVerts;
                for (const auto& p : particles) {
                    pVerts.push_back({ p.position, glm::vec3(0), glm::vec2(0) });
                }

                Mesh particleMesh(pVerts, GL_POINTS);
                shader->setUniform("model", glm::mat4(1.0f));
                glPointSize(8.0f);
                particleMesh.draw();
            }


            // 3. VYKRESLENÍ PRŮHLEDNÝCH MODELŮ (Task 1)
            glDepthMask(GL_FALSE); // Vypneme zapis do Z-bufferu
            shader->setUniform("objectAlpha", 0.4f); // 40% neprůhlednost

            for (int i = 0; i < 3; i++) {
                glm::mat4 transModel = glm::mat4(1.0f);
                transModel = glm::translate(transModel, glm::vec3(3.0f, 0.0f, -2.0f - (i * 2.0f)));
                shader->setUniform("model", transModel);
                myModel->draw();
            }

            glDepthMask(GL_TRUE); // Znovu zapneme pro dalsi snimek

            // ==========================================

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