#pragma once

#include <GL/glew.h>
#include <string>

class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    void use() const { glUseProgram(program_); }
    GLint uniform(const char* name) const;

private:
    GLuint program_ = 0;
};
