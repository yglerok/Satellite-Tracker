#pragma once

#include <ctime>
#include <chrono>

#include <glad/glad.h>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

#include "../TimeManager.h"

const double DEG_TO_RAD = M_PI / 180.0;
const double RAD_TO_DEG = 180.0 / M_PI;

class Sun
{
public:
	Sun(std::shared_ptr<TimeManager> timeManager);
	~Sun() = default;

	void setLightning(GLuint shaderProgram);
	glm::vec3 getDirection(); // Получение вектора направления на Солнце

private:
	//bool getLocalTime(const time_t* time, struct tm* result); // кроссплатформенная реализация localtime()
	double getEclipticLongitude(const double& jd); // Расчёт эклиптической долготы Солнца
	void getEquatorialCoords(double lambda, double beta, double epsilon,
		double& ra, double& dec); // Переход в экваториальные координаты
	
	const float distance = 1496.0f;

	std::shared_ptr<TimeManager> timeManager;
};