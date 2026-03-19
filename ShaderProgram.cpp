#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "ShaderProgram.hpp"
#include "Mesh.hpp" 

ShaderProgram::ShaderProgram(const std::string& vertex_shader_code, const std::string& fragment_shader_code) {
    // compile shaders and store IDs for linker
    auto vertex_shader = compile_shader(vertex_shader_code, GL_VERTEX_SHADER);
    auto fragment_shader = compile_shader(fragment_shader_code, GL_FRAGMENT_SHADER);

    std::vector<GLuint> shader_ids{ vertex_shader, fragment_shader };

    // link all compiled shaders into shader program 
    ID = link_shader(shader_ids);
}

// FIX 1: Use textFileRead instead of read_text_file
ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file) :
    ShaderProgram{ textFileRead(VS_file), textFileRead(FS_file) } {
}

// FIX 4: Changed return type from GLuint to GLint to properly handle the -1 error code
GLint ShaderProgram::getUniformLocation(const std::string& name) {
    // Check if the location is already cached
    if (uniform_location_cache.contains(name)) { // C++20 feature
        return uniform_location_cache[name];
    }

    // Get the location and cache it
    GLint loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "No uniform with name: " << name << '\n';
    }
    else {
        uniform_location_cache[name] = loc;
    }
    return loc;
}

GLint ShaderProgram::getAttribLocation(const std::string& name) {
    // FIX 2: Added .c_str()
    GLint loc = glGetAttribLocation(ID, name.c_str());
    if (loc == -1) {
        std::cerr << "No vertex attribute with name: " << name << ", or reserved name (starting with gl_)\n";
    } // FIX 2: Added missing closing bracket
    return loc;
}

// Uniform setting
void ShaderProgram::setUniform(const std::string& name, const GLfloat val) {
    auto loc = getUniformLocation(name);
    glProgramUniform1f(ID, loc, val);
}

// FIX 3: Changed in_vec4 to val to match the function body
void ShaderProgram::setUniform(const std::string& name, const glm::vec4& val) {
    auto loc = getUniformLocation(name);
    glProgramUniform4fv(ID, loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3& val) {
    auto loc = getUniformLocation(name);
    glProgramUniformMatrix3fv(ID, loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const std::vector<GLint>& val) {
    auto loc = getUniformLocation(name);
    glProgramUniform1iv(ID, loc, val.size(), val.data());
}

void ShaderProgram::setUniform(const std::string& name, const std::vector<glm::vec3>& val) {
    auto loc = getUniformLocation(name);
    glProgramUniform3fv(ID, loc, val.size(), glm::value_ptr(val[0]));
}

std::string ShaderProgram::getShaderInfoLog(const GLuint obj) {
    int log_length = 0;
    std::string s;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> v(log_length);
        glGetShaderInfoLog(obj, log_length, nullptr, v.data());
        s.assign(begin(v), end(v));
    }
    return s;
}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
    int log_length = 0;
    std::string s;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        std::vector<char> v(log_length);
        glGetProgramInfoLog(obj, log_length, nullptr, v.data());
        s.assign(begin(v), end(v));
    }
    return s;
}

GLuint ShaderProgram::compile_shader(const std::string& source_code, const GLenum type) {
    char const* src_cstr = source_code.c_str();

    GLuint shader_ID = glCreateShader(type);

    glShaderSource(shader_ID, 1, &src_cstr, nullptr);
    glCompileShader(shader_ID);
    {
        GLint status;
        glGetShaderiv(shader_ID, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            std::cerr << getShaderInfoLog(shader_ID) << std::endl;
            glDeleteShader(shader_ID);
            throw std::runtime_error("Shader compilation failed.");
        }
    }

    return shader_ID;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
    GLuint prog_ID = glCreateProgram();

    for (const auto& id : shader_ids)
        glAttachShader(prog_ID, id);

    // force OpenGL to use specific slots(locations) for certain vertex attributes,
    // must be set before linking
    glBindAttribLocation(prog_ID, Mesh::attribute_location_position, "position");
    glBindAttribLocation(prog_ID, Mesh::attribute_location_normal, "normal");
    glBindAttribLocation(prog_ID, Mesh::attribute_location_texture_coords, "texture_coords");

    glLinkProgram(prog_ID);

    for (const auto& id : shader_ids) {
        glDetachShader(prog_ID, id);
        glDeleteShader(id);
    }

    // check link result, print info & throw error (if any)
    {
        GLint status;
        glGetProgramiv(prog_ID, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            std::cerr << "Error linking shader program." << std::endl;
            std::cerr << getProgramInfoLog(prog_ID) << std::endl;
            glDeleteProgram(prog_ID);
            throw std::runtime_error("Shader linking failed.");
        }
    }
    return prog_ID;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open())
        throw std::runtime_error(std::string("Error opening file: ") + filepath.string());
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
void ShaderProgram::setUniform(const std::string& name, const glm::mat4& val) {
    auto loc = getUniformLocation(name);
    glProgramUniformMatrix4fv(ID, loc, 1, GL_FALSE, glm::value_ptr(val));
}

// You are also missing the glm::vec3 implementation, which you might need later:
void ShaderProgram::setUniform(const std::string& name, const glm::vec3& val) {
    auto loc = getUniformLocation(name);
    glProgramUniform3fv(ID, loc, 1, glm::value_ptr(val));
}