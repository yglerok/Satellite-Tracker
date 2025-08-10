#pragma once
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

enum ShaderTypes {
	VERTEX, FRAGMENT, PROGRAM
};

class Shader
{
public:
	static GLuint create(const std::string& vertexPath, const std::string& fragmentPath);

	// ������� ��� �������� ������ � ������
	static void setMat4(GLuint shader, const std::string& name, const glm::mat4& matrix);
	static void setVec3(GLuint shader, const std::string& name, const glm::vec3& vector);
	

private:
	static std::string readFile(const std::string& path);
	// �������� ������ ����������
	static void checkCompileErrors(GLuint shader, ShaderTypes type);
};