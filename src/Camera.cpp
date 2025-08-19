#include "Camera.h"

#include <iostream>

Camera::Camera(int width, int height)
{
	view = glm::lookAt(
		position.current,	// ������� ������
		glm::vec3(0.0f, 0.0f, 0.0f),	// ����������� ������� (�����, � ������� ������� ������)
		glm::vec3(0.0f, 1.0f, 0.0f)		// ������ "�����" (������ (0, 1, 0))
	);

	projection = glm::perspective(
		glm::radians(45.0f),	// ���� ������ (FOV) (45-90 ����)
		float(width) / float(height),		// ����������� ������ (������ / ������)
		0.1f,					// ������� ��������� ��������� 
		100.0f					// ������� ��������� ���������
	);
}

void Camera::increasePhi(float deltaPhi)
{
	phi.target -= deltaPhi;
}

void Camera::increaseTheta(float deltaTheta)
{
	theta.target += deltaTheta;
	// ������������ theta, ����� ����������� ��������� ������
	theta.target = glm::clamp(theta.target, -1.4f, 1.4f);
}

void Camera::increaseRadius(float deltaR)
{
	radius.target -= deltaR * 0.5;
	radius.target = glm::clamp(radius.target, 3.0f, 15.0f);
}

void Camera::reset()
{
	phi.target = 0.0f;
	theta.target = 0.0f;
	radius.target = 5.0f;
}

void Camera::update(double dt)
{
	phi.current = glm::mix(phi.current, phi.target,
		rotationInterpolationSpeed * dt * 60.0f);
	theta.current = glm::mix(theta.current, theta.target,
		rotationInterpolationSpeed * dt * 60.0f);
	radius.current = glm::mix(radius.current, radius.target,
		scaleInterpolationSpeed * dt * 60.0f);

	float camX = cos(theta.current) * sin(phi.current) * radius.current;
	float camZ = cos(theta.current) * cos(phi.current) * radius.current;
	float camY = sin(theta.current) * radius.current;

	position.previous = position.current;
	position.current = glm::vec3(camX, camY, camZ);
}

void Camera::render(double alpha)
{
	// ������������ ������� ������ ��� ���������
	position.render = position.previous +
		(position.current - position.previous) * float(alpha);

	// ��������� view ������� � ����������������� ��������
	view = glm::lookAt(
		position.render,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}
