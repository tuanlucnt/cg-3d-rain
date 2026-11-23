#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
GLuint compile(GLenum type, const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open shader: " + path);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    if (file.bad()) {
        throw std::runtime_error("Cannot read shader: " + path);
    }
    const std::string source = buffer.str();
    const char* data = source.c_str();
    const GLuint shader = glCreateShader(type);
    if (!shader) {
        throw std::runtime_error("glCreateShader failed: " + path);
    }
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
        std::vector<char> log(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
        glGetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compilation failed (" + path + "):\n" + log.data());
    }
    return shader;
}
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment = 0;
    try {
        fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);
        program_ = glCreateProgram();
        if (!program_) {
            throw std::runtime_error("glCreateProgram failed");
        }
        glAttachShader(program_, vertex);
        glAttachShader(program_, fragment);
        glLinkProgram(program_);
        GLint success = GL_FALSE;
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            GLint size = 0;
            glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &size);
            std::vector<char> log(static_cast<std::size_t>(size > 0 ? size : 1), '\0');
            glGetProgramInfoLog(program_, static_cast<GLsizei>(log.size()), nullptr, log.data());
            throw std::runtime_error(std::string("Shader linking failed:\n") + log.data());
        }
    } catch (...) {
        glDeleteProgram(program_);
        program_ = 0;
        glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        throw;
    }
    glDetachShader(program_, vertex);
    glDetachShader(program_, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() { glDeleteProgram(program_); }

GLint Shader::uniform(const char* name) const {
    const GLint location = glGetUniformLocation(program_, name);
    if (location < 0) {
        throw std::runtime_error(std::string("Missing active shader uniform: ") + name);
    }
    return location;
}
