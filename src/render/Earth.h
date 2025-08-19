#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Vertex {
	glm::vec3 position;	// позиция (x, y, z) 
	glm::vec2 uv;		// текстурные координаты (u, v)
	glm::vec3 normal;	// нормаль (для освещения)
};

class Earth
{
public:
	Earth();
	Earth(Earth&) = delete;
	~Earth();
	
	void render(const glm::mat4& view, const glm::mat4& projection, GLuint shader) const;

private:
	std::vector<Vertex> vertices; 
	std::vector<unsigned int> indices;
	GLuint textureDay;
	GLuint textureNight;
	GLuint vao, vbo, ebo;

	int segments = 64;

	void loadTexture(const std::string& texturePath, GLuint& texture);
	void genarateSphereVertices();

	GLuint meridianVAO, meridianVBO;
	std::vector<glm::vec3> meridianVertices;
	GLuint lineShader;
	void generateMeridianVertices();

	glm::mat4 model = glm::mat4(1.0f); // матрица модели
};