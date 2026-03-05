#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <exception>
#include <memory> // Required for std::unique_ptr

#include "../ShaderProgram.hpp"
#include "../Mesh.hpp"

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

    // --- Window & Input State ---
    bool isCursorCaptured = false;
    bool isFullscreen = false;
    int prevWinPos[2] = { 0, 0 };
    int prevWinSize[2] = { 800, 600 };

    // Colors
    float bgColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
    float triangleColor[4] = { 1.0f, 0.5f, 0.2f, 1.0f };

    // --- Modular Resources ---
    std::unique_ptr<ShaderProgram> shader;
    std::unique_ptr<Mesh> myModel;

    // --- Multi-monitor helper ---
    GLFWmonitor* getCurrentMonitor(GLFWwindow* window);

    // --- Static Callbacks ---
    static void glfw_error_callback(int error, const char* description);
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
        GLenum severity, GLsizei length,
        const char* message, const void* userParam);

    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};