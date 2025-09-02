#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <iostream>

#include "../third_party/sgp4/src/cpp/SGP4.h"

#include "../data/TleParser.h"

// Использую паттерн объект-значение:
// объект характеризуется своим состоянием, неизменяем после создания (если изменить - это будет уже другой объект)
// объекты равны, если равны их свойства и значения, независимо от ссылок на них
class Sgp4Model
{
public:
	// Для управления жизненным циклом будем использовать shared_ptr
	Sgp4Model(const Sgp4Model&) = delete;
	Sgp4Model& operator=(const Sgp4Model&) = delete;

	~Sgp4Model() = default;

	// Единственный способ создать модель - через фабрику, использующую этот конструктор.
	Sgp4Model(const std::string& line1, const std::string& line2);

	// Основной метод: рассчитать положение и скорость на заданное время (в минутах от эпохи TLE).
	bool calcPosition(double minutesSinceEpoch, glm::dvec3& outPosition, glm::dvec3& outVelocity) const;

	const std::string& getTleLine1() const { return tleLine1; }
	const std::string& getTleLine2() const { return tleLine2; }
	int getNoradId() const;
	bool isValid() const { return isInitialized; }
	double getEpochJulianDate() const;

protected:
	bool initialize();

private:
	

	std::string tleLine1;
	std::string tleLine2;
	std::unique_ptr<ElsetRec> elements;
	bool isInitialized = false;
};