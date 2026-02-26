#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <exception>

class App {
public:
    App();
    ~App();

    bool init();
    int run();

private:
    GLFWwindow* window = nullptr;

    // --- Application State ---
    bool vsyncEnabled = true;
    int fps = 0;
    int winWidth = 800;
    int winHeight = 600;

    // --- Window & Input State (Added for Tasks 1 & 2) ---
    bool isCursorCaptured = false;
    bool isFullscreen = false;
    int prevWinPos[2] = { 0, 0 };
    int prevWinSize[2] = { 800, 600 };

    // Colors
    float bgColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
    float triangleColor[4] = { 1.0f, 0.5f, 0.2f, 1.0f };

    // Placeholder for your shader uniform (set this after you compile shaders)
    GLuint uniform_color_location = 0;

    // --- Multi-monitor helper (Added for Task 2) ---
    GLFWmonitor* getCurrentMonitor(GLFWwindow* window);

    // --- Static Callbacks ---
    static void glfw_error_callback(int error, const char* description);
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
        GLenum severity, GLsizei length,
        const char* message, const void* userParam);

    // Lab 02 Task 3: All required input callbacks
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    GLuint shaderProgram = 0;
    GLuint VAO = 0;
    GLuint VBO = 0;
};