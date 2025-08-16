#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <ctime>
#include <chrono>

#include <glm/glm.hpp>
#include <glad/glad.h>

const double DEG_TO_RAD = M_PI / 180.0;
const double RAD_TO_DEG = 180.0 / M_PI;

class Sun
{
public:
	Sun() = default;
	~Sun() = default;

	void setLightning(GLuint shaderProgram);

private:
	bool getLocalTime(const time_t* time, struct tm* result); // кроссплатформенная реализация localtime()
	std::tm getUtcTime( time_t time);
	double getJulianDate(); // Расчёт юлианской даты (JD)
	double getEclipticLongitude(const double& mjd); // Расчёт эклиптической долготы Солнца
	void getEquatorialCoords(double lambda, double beta, double epsilon,
		double& ra, double& dec); // Переход в экваториальные координаты
	glm::vec3 getDirection(); // Получение вектора направления на Солнце
	const float distance = 1496.0f;
};