#pragma once
#include <string>
#include <filesystem>
#include <unordered_map>
#include <vector> // FIX: Added missing include for std::vector
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include "non_copyable.hpp"

class ShaderProgram : private NonCopyable {
public:
    // No default constructor. RAII - if constructed, it will be correctly initialized
    // and can be rendered. OpenGL resources are guaranteed to be deallocated using destructor. 
    // Double-free errors are prevented by making class non-copyable (therefore 
    // double destruction of the same OpenGL shader is prevented). 
    ShaderProgram(void) = delete; //does nothing

    // you can add more constructors for pipeline with GS, TS etc.
    ShaderProgram(std::string const& vertex_shader_code, std::string const& fragment_shader_code);
    ShaderProgram(std::filesystem::path const& VS_file, std::filesystem::path const& FS_file);

    // activate shader
    void use(void) {
        // FIX: Changed currently_used_ID to currently_used to match the member variable
        if (ID == currently_used) // already being used
            return;
        else {
            glUseProgram(ID);
            currently_used = ID;
        }
    };

    // deactivate current shader program (i.e. activate shader no. 0)
    void deactivate(void) {
        glUseProgram(0);
        currently_used = 0;
    };

    ~ShaderProgram(void) {  //deallocate shader program
        deactivate();
        glDeleteProgram(ID);
        ID = 0;
    }

    GLuint getID(void) { return ID; }
    GLint  getAttribLocation(const std::string& name);

    // set uniform according to name 
    // https://docs.gl/gl4/glUniform
    void setUniform(const std::string& name, const GLfloat val);
    void setUniform(const std::string& name, const GLint val);
    void setUniform(const std::string& name, const glm::vec3& val);
    void setUniform(const std::string& name, const glm::vec4& val);
    void setUniform(const std::string& name, const glm::mat3& val);
    void setUniform(const std::string& name, const glm::mat4& val);
    void setUniform(const std::string& name, const std::vector<GLint>& val);
    void setUniform(const std::string& name, const std::vector<GLfloat>& val);
    void setUniform(const std::string& name, const std::vector<glm::vec3>& val);

private:
    GLuint ID{ 0 }; // default = 0, empty shader
    inline static GLuint currently_used{ 0 };

    // FIX: Changed GLuint to GLint to handle the -1 error state
    std::unordered_map<std::string, GLint> uniform_location_cache;

    // FIX: Changed return type from GLuint to GLint
    GLint getUniformLocation(const std::string& name);

    std::string textFileRead(const std::filesystem::path& filename); // load text file

    GLuint compile_shader(const std::string& source_code, const GLenum type);
    std::string getShaderInfoLog(const GLuint obj);

    GLuint link_shader(const std::vector<GLuint> shader_ids);
    std::string getProgramInfoLog(const GLuint obj);
};