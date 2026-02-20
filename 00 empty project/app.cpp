// icp.cpp // author: JJ
#include "app.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

using json = nlohmann::json;

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

    // Task 2: Quit handling
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Task 4: Toggle VSync
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        app->vsyncEnabled = !app->vsyncEnabled;
        glfwSwapInterval(app->vsyncEnabled ? 1 : 0);
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // Task 3: Change background color on click
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        app->bgColor[0] = (rand() % 100) / 100.0f;
        app->bgColor[1] = (rand() % 100) / 100.0f;
        app->bgColor[2] = (rand() % 100) / 100.0f;
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    // Optional: map cursor X position to the Red channel of the triangle
    // app->triangleColor[0] = (float)(xpos / app->winWidth);
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    // Task 3: use scroll wheel to change the Blue channel of the triangle
    app->triangleColor[2] += (float)yoffset * 0.1f;
    if (app->triangleColor[2] > 1.0f) app->triangleColor[2] = 1.0f;
    if (app->triangleColor[2] < 0.0f) app->triangleColor[2] = 0.0f;
}

// --- Class Methods ---

App::App() { std::cout << "Constructed...\n"; }

bool App::init() {
    try {
        // Task 2: Safe GLFW Init
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

        // Task 4: Use JSON config
        std::ifstream f("config.json");
        if (f.is_open()) {
            json data = json::parse(f);
            winWidth = data.value("width", 800);
            winHeight = data.value("height", 600);
        }
        else {
            std::cerr << "config.json not found. Using default resolution.\n";
        }

        // Task 1: Check for debug extension / Context 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(winWidth, winHeight, "OpenGL Lab", nullptr, nullptr);
        if (!window) throw std::runtime_error("Failed to create GLFW window");

        glfwMakeContextCurrent(window);

        // GLEW Init
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to initialize GLEW");
        glGetError();

        glfwSetWindowUserPointer(window, this);

        // Task 3: Register all callbacks
        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, fbsize_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // Task 1: Activate debug output
        int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        glfwSwapInterval(vsyncEnabled ? 1 : 0);

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

        // Build and compile shader program
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // (In a real engine, check for compilation errors here)

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // (In a real engine, check for compilation errors here)

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // (In a real engine, check for linking errors here)

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Set up vertex data (and buffer(s)) and configure vertex attributes
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

        // Get the uniform location for ourColor
        uniform_color_location = glGetUniformLocation(shaderProgram, "ourColor");

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
            // Task 3 & 4: Measure FPS and Update Title with VSync status
            double currentTime = glfwGetTime();
            frameCount++;
            if (currentTime - previousTime >= 1.0) {
                fps = frameCount;
                frameCount = 0;
                previousTime = currentTime;

                std::string title = "FPS: " + std::to_string(fps) + " | VSync: " + (vsyncEnabled ? "ON" : "OFF");
                glfwSetWindowTitle(window, title.c_str());
            }

            // Task 3: Clear canvas with preset background color
            glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ==========================================
            // RENDER TRIANGLE
            // ==========================================

            // 1. Tell OpenGL to use the shader program BEFORE setting uniforms
            glUseProgram(shaderProgram);

            // Task 3: Change triangle color using glfwGetTime() (Green channel)
            float timeValue = glfwGetTime();
            triangleColor[1] = (sin(timeValue) / 2.0f) + 0.5f;

            // 2. Apply the color to the shader uniform
            if (uniform_color_location != -1) {
                glUniform4f(uniform_color_location, triangleColor[0], triangleColor[1], triangleColor[2], triangleColor[3]);
            }

            // 3. Bind the VAO and draw
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

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
    // Cleanup OpenGL resources
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);

    if (window) glfwDestroyWindow(window);
    glfwTerminate();
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}