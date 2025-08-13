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
	
	void render(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint shader) const;

private:
	std::vector<Vertex> vertices; 
	std::vector<unsigned int> indices;
	GLuint textureDay;
	GLuint textureNight;
	GLuint vao, vbo, ebo;

	void loadTexture(const std::string& texturePath, GLuint& texture);
	void genarateSphereVertices(int segments = 64);
};