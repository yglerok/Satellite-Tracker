#include "Shaders.h"
#include <fstream>
#include <sstream>
#include <iostream>

GLuint Shader::create(const std::string& vertexPath, const std::string& fragmentPath)
{
    // чтение исходников
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);
    const char* vcode = vertexCode.c_str();
    const char* fcode = fragmentCode.c_str();

    // компил€ци€ вершинного шейдера
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vcode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, VERTEX);

    // компил€ци€ фрагментного шейдера
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fcode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, FRAGMENT);

    // линковка программы
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    checkCompileErrors(program, PROGRAM);

    // удаление шейдеров
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

void Shader::setMat4(GLuint shader, const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(glGetUniformLocation(shader, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::setVec3(GLuint shader, const std::string& name, const glm::vec3& vector)
{
    glUniform3fv(glGetUniformLocation(shader, name.c_str()), 1, glm::value_ptr(vector));
}

void Shader::checkCompileErrors(GLuint shader, ShaderTypes type)
{
    GLint success;
    GLchar infoLog[1024];

    if (type != PROGRAM) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "Shader compile error (" << type << "):\n" <<
                infoLog << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "Shader program compile error:\n" << infoLog << std::endl;
        }
    }
}

std::string Shader::readFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Can't open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
