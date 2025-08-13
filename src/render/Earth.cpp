#include "Earth.h"
#include "Shaders.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>

Earth::Earth()
{
    genarateSphereVertices();
    loadTexture("res/textures/earth.jpg");

    // создание vao, vbo
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(vao);
    
    // Заполняем vbo вершинами
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // заполняем ebo индексами
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
        indices.data(), GL_STATIC_DRAW);

    // Атрибуты вершин 
    // позиция (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // UV-координаты (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    // Нормали (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);
}

Earth::~Earth()
{
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void Earth::loadTexture(const std::string& texturePath)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Настройки фильтрации
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Загрузка изображения
    int width, height, channels;
    
    unsigned char* data = stbi_load(texturePath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "Can't load texture: " << texturePath << std::endl;
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
        channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

void Earth::genarateSphereVertices(int segments)
{
    // Генерация вершин
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments; // [0, pi]
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2 * M_PI / segments; // [0, 2pi]

            Vertex v;
            // Позиция (сферические координаты -> декартовы
            v.position = glm::vec3(
                sin(theta) * cos(phi),
                cos(theta),
                sin(theta) * sin(phi)
            );
            // UV-координаты 
            v.uv = glm::vec2(
                1.0f - static_cast<float>(lon) / segments,
                static_cast<float>(lat) / segments
            );
            // нормаль = нормализованная позиция
            v.normal = glm::normalize(v.position);

            vertices.push_back(v);
        }
    }

    // Генерация индексов
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = (lat * (segments + 1)) + lon;
            int second = first + segments + 1;

            // k1 -- k1+1
            // |   /  |
            // |  /   |
            // k2 -- k2+1
            // порядок против час. стрелки

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);
            
            indices.push_back(first + 1);
            indices.push_back(second + 1);
            indices.push_back(second);
            
            
        }
    }
}

void Earth::render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint shader) const
{
    glUseProgram(shader);

    glm::vec3 viewPos = glm::vec3(-view[3][0], -view[3][1], -view[3][2]);

    // Передача матриц в шейдер
    Shader::setMat4(shader, "model", model);
    Shader::setMat4(shader, "view", view);
    Shader::setMat4(shader, "projection", projection);
    Shader::setVec3(shader, "viewPos", viewPos);

    // Привязка текстуры
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "earthTexture"), 0);

    // Отрисовка 
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
