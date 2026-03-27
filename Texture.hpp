// Texture.hpp
#pragma once 

#include <filesystem>
#include <opencv2/opencv.hpp>
#include <GL/glew.h> 
#include <glm/glm.hpp>

#include "non_copyable.hpp"

class Texture : private NonCopyable {
public:
    enum class Interpolation {
        nearest,
        linear,
        linear_mipmap_linear,
    };

    Texture() = delete;
    Texture(const cv::Mat& image, Interpolation interpolation = Interpolation::linear_mipmap_linear); // default = best texture filtering
    Texture(const glm::vec3& vec); // synthetic single-color RGB texture
    Texture(const glm::vec4& vec); // synthetic single-color RGBA texture
    Texture(const std::filesystem::path& path, Interpolation interpolation = Interpolation::linear_mipmap_linear);

    ~Texture();

    void bind(void);
    GLuint get_name() const;
    int get_height(void);
    int get_width(void);
    void set_interpolation(Interpolation interpolation);
    void replace_image(const cv::Mat& image);
private:
    cv::Mat load_image(const std::filesystem::path& path);
    GLuint name_{ 0 };
};