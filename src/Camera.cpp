#include "Camera.h"

#include <iostream>

Camera::Camera(int width, int height)
{
	view = glm::lookAt(
		position.current,	// позиция камеры
		glm::vec3(0.0f, 0.0f, 0.0f),	// направление взгляда (точка, в которую смотрит камера)
		glm::vec3(0.0f, 1.0f, 0.0f)		// вектор "вверх" (обычно (0, 1, 0))
	);

	projection = glm::perspective(
		glm::radians(45.0f),	// угол обзора (FOV) (45-90 град)
		float(width) / float(height),		// соотношение сторон (ширина / высота)
		0.1f,					// ближняя плоскость отсечения 
		100.0f					// дальняя плоскость отсечения
	);
}

void Camera::increasePhi(float deltaPhi)
{
	phi.target -= deltaPhi;
}

void Camera::increaseTheta(float deltaTheta)
{
	theta.target += deltaTheta;
	// ограничиваем theta, чтобы недопустить переворот камеры
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
	// Интерполяция позиции камеры для плавности
	position.render = position.previous +
		(position.current - position.previous) * float(alpha);

	// Обновляем view матрицу с интерполированной позицией
	view = glm::lookAt(
		position.render,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}
