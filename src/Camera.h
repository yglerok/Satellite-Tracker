#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera(int width, int height);
	~Camera() = default;

	void increasePhi(float deltaPhi);
	void increaseTheta(float deltaTheta);
	void increaseRadius(float deltaR);

	void reset();
	void update(double dt); // ��������� ������� ������ � ������������ � ������������ ������
	void render(double alpha); // ��������� ������� ����

	glm::mat4 getProjection() const { return projection; }
	glm::mat4 getView() const { return view; }

private:
	struct Position {
		glm::vec3 previous = glm::vec3(5.0f, 0.0f, 5.0f);
		glm::vec3 current = glm::vec3(5.0f, 0.0f, 5.0f);
		glm::vec3 render = glm::vec3(5.0f, 0.0f, 5.0f);
	} position;
	
	// ������� ����
	glm::mat4 view;
	// ������ ������������� ������� (��������� ������������ ������)
	glm::mat4 projection;

	// ��������� ��������� ������
	struct Phi {
		float current = 0.0f;
		float target = 0.0f;
	} phi;
	struct Theta {
		float current = 0.0f;
		float target = 0.0f;
	} theta;
	struct Radius {
		float current = 5.0f;
		float target = 5.0f;
	} radius;

	const float rotationInterpolationSpeed = 0.1f; // � �������
	const float scaleInterpolationSpeed = 0.05f;
};