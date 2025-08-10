#include "Earth.h"
#include "Shaders.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>

Earth::Earth()
{
    genarateSphereVertices();
    loadTexture();

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
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

bool Earth::loadTexture()
{
    return false;
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
                static_cast<float>(lon) / segments,
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
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void Earth::render(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const
{
    glUseProgram(shader);

    // Матрица модели (вращение земли)
    /*glm::mat4 model = glm::rotate(glm::mat4(1.0f), (float)SDL_GetTicks() / 1000.0f,
        glm::vec3(0.0f, 1.0f, 0.0f));*/
    glm::mat4 model = glm::mat4(1.0f);

    // Передача матриц в шейдер
    Shader::setMat4(shader, "model", model);
    Shader::setMat4(shader, "view", view);
    Shader::setMat4(shader, "projection", projection);

    // Привязка текстуры
    Shader::setVec3(shader, "objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
    Shader::setVec3(shader, "lightPos", glm::vec3(2.0f, 3.0f, 3.0f));
    Shader::setVec3(shader, "lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // Отрисовка 
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
