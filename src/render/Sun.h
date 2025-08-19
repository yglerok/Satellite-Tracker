#pragma once

#include <ctime>
#include <chrono>

#include <glad/glad.h>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

const double DEG_TO_RAD = M_PI / 180.0;
const double RAD_TO_DEG = 180.0 / M_PI;

class Sun
{
public:
	Sun() = default;
	~Sun() = default;

	void setLightning(GLuint shaderProgram);

private:
	bool getLocalTime(const time_t* time, struct tm* result); // ������������������ ���������� localtime()
	std::tm getUtcTime( time_t time);
	double getJulianDate(); // ������ ��������� ���� (JD)
	double getEclipticLongitude(const double& mjd); // ������ ������������� ������� ������
	void getEquatorialCoords(double lambda, double beta, double epsilon,
		double& ra, double& dec); // ������� � �������������� ����������
	glm::vec3 getDirection(); // ��������� ������� ����������� �� ������
	const float distance = 1496.0f;
};