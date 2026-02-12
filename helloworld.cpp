#include <iostream>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//#include <opencv4/opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

int main()
{
    std::cout << "Hello!\n";

    // ===  important for 2D raster graphics  ===

        // demo: use OpenCV
    cv::Mat x(cv::Size(100, 100), CV_8UC3);
    cv::namedWindow("test_window");

    // try to open first camera of any type and grab single image
    auto cam = cv::VideoCapture(0, cv::CAP_ANY);
    if (cam.isOpened())
        cam.read(x);

    cv::imshow("test_window", x);
    cv::pollKey();


    // ===  important for 3D graphics  ===

        // demo: use GLM
    glm::vec3 vec{};
    std::cout << glm::to_string(vec) << '\n';

    // demo: use glfw
    if (!glfwInit()) exit(100);
    GLFWwindow* w = glfwCreateWindow(800, 600, "test", nullptr, nullptr);
    if (!w) exit(111);
    glfwMakeContextCurrent(w);

    // demo: use glew
    auto s = glewInit();
    if (s != GLEW_OK) exit(123);

    // wait for ESC key in GLFW window
    while (!glfwWindowShouldClose(w)) {
        if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(w, GLFW_TRUE);
        }

        glfwPollEvents();
    }

    std::cout << "Bye!\n";
    return 0;
}