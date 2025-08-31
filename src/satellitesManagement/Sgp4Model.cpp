#include "Sgp4Model.h"

#include <iostream>

Sgp4Model::Sgp4Model(const std::string& line1, const std::string& line2) :
	tleLine1(line1), tleLine2(line2), elements(std::make_unique<ElsetRec>())
{
    // Заполняем сырые поля ElsetRec из строк TLE.
    // ЭТО ВАЖНЕЙШАЯ ЧАСТЬ! Здесь должен быть парсинг TLE-строк.
    // Это сложно и требует отдельной функции. Для примера - заглушка.

    // ПРИМЕР (заглушка)! Вам нужно реализовать полноценный парсинг!
    // m_elements->inclo = ... извлечь из line2;
    // m_elements->ecco = ... извлечь из line2;
    // m_elements->no_kozai = ... извлечь из line2;
    // ... и так для всех необходимых полей.

    // После заполнения полей инициализируем модель SGP4.
    isInitialized = initialize();
}

bool Sgp4Model::calcPosition(double minutesSinceEpoch, glm::dvec3& outPosition, glm::dvec3& outVelocity) const
{
    if (!isInitialized) {
        std::cerr << "Can't calculate position, Sgp4Model is not initialized!" << std::endl;
        return false;
    }

    double position[3];
    double velocity[3];

    // Создаем копию ElsetRec для расчета (так как sgp4 может менять некоторые внутренние поля)
    ElsetRec tempRec = *elements;

    if (sgp4(&tempRec, minutesSinceEpoch, position, velocity)) {
        outPosition = glm::dvec3(position[0], position[1], position[2]);
        outVelocity = glm::dvec3(velocity[0], velocity[1], velocity[2]);
        return true;
    }
    else {
        std::cerr << "SGP4 calculation failed for NORAD ID: " << getNoradId() << " at time " << minutesSinceEpoch << " min." << std::endl;
        std::cerr << "Error code: " << tempRec.error << std::endl;
        return false;
    }
}

int Sgp4Model::getNoradId() const
{
    return TleParser::extractNoradIdFromLine2(tleLine2);
}

bool Sgp4Model::initialize()
{
    getgravconst(wgs84, elements.get());

    if (!sgp4init('a', elements.get())) {
        std::cerr << "SGP4 Init failed for TLE: " << tleLine1 << std::endl;
        if (elements->error != 0)
            std::cerr << "Error code: " << elements->error << std::endl;
        return false;
    }
    return true;
}
