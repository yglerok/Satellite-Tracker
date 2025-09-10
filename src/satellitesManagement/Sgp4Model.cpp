#include "Sgp4Model.h"
#include "../TimeManager.h"

#include <charconv> // Для std::from_chars (C++17)
#include <cstring>  // Для std::strlen
#include <iostream>

// Вспомогательная функция для парсинга чисел и дробей из строки TLE
static double parseTleDouble(const char* str, int start, int length, double defaultVal = 0.0) {
    // Буфер для копирования подстроки
    char buffer[16] = { 0 };
    std::strncpy(buffer, str + start, length);
    buffer[length] = '\0';

    // Проверка на пустую строку или строку из пробелов
    if (std::strlen(buffer) == 0) {
        return defaultVal;
    }

    double result;
    // Пытаемся распарсить как double
    auto [ptr, ec] = std::from_chars(buffer, buffer + std::strlen(buffer), result);

    if (ec == std::errc()) {
        return result;
    }
    else {
        // Если не получилось, возможно, число записано в форме "12345-3" (0.12345e-3)
        // Это упрощенная обработка, настоящий парсер должен быть надежнее
        char* endptr;
        result = std::strtod(buffer, &endptr);
        if (endptr != buffer) {
            return result;
        }
    }
    std::cerr << "TLE Parser: Failed to parse number from '" << buffer << "'" << std::endl;
    return defaultVal;
}

static double parseTleExpNumber(const char* str, int start, int length, bool isBstar = false)
{
    char buffer[16] = { 0 };
    std::strncpy(buffer, str + start, length);
    buffer[length] = '\0';

    // Удаляем все пробелы
    std::string numStr(buffer);
    numStr.erase(std::remove(numStr.begin(), numStr.end(), ' '), numStr.end());

    if (numStr.empty()) 
        return 0.0;

    if (numStr.find('.') == std::string::npos) {
        numStr = "0." + numStr;
    }
    else if (numStr[0] == '.') {
        numStr = "0" + numStr;
    }

    // Ищем знак экспоненты (может быть в любой позиции кроме первой)
    size_t minusPos = numStr.find('-', 1);  // Ищем минус не на первой позиции
    size_t plusPos = numStr.find('+', 1);   // Ищем плюс не на первой позиции

    if (minusPos != std::string::npos) {
        std::string baseStr = numStr.substr(0, minusPos);
        std::string expStr = numStr.substr(minusPos + 1);

        double base = std::strtod(baseStr.c_str(), nullptr);
        int exponent = std::atoi(expStr.c_str());
        return base * std::pow(10.0, -exponent);
    }
    else if (plusPos != std::string::npos) {
        std::string baseStr = numStr.substr(0, plusPos);
        std::string expStr = numStr.substr(plusPos + 1);

        double base = std::strtod(baseStr.c_str(), nullptr);
        int exponent = std::atoi(expStr.c_str());
        return base * std::pow(10.0, exponent);
    }
    else {
        return std::strtod(numStr.c_str(), nullptr);
    }
}

Sgp4Model::Sgp4Model(const std::string& line1, const std::string& line2) :
	tleLine1(line1), tleLine2(line2), elements(std::make_unique<ElsetRec>())
{
    // Инициализируем структуру нулями
    std::memset(elements.get(), 0, sizeof(ElsetRec));

    const char* l1 = tleLine1.c_str();
    const char* l2 = tleLine2.c_str();

    // === ПАРСИНГ ПЕРВОЙ СТРОКИ TLE ===
    // 1. Номер строки (пропускаем)
    // 2. Номер каталога (Satellite number) - позиции 3-7
    std:strncpy(elements->satid, l1 + 2, 5);
    elements->satid[5] = '\0';

    // 3. Классификация (Classification) - позиция 8
    elements->classification = l1[7];

    // 4. Международный идентификатор (International Designator) - позиции 10-17
    std::strncpy(elements->intldesg, l1 + 9, 8);
    elements->intldesg[8] = '\0';

    // 5. Эпоха (Epoch) - позиции 19-32 (Год и день с дробной частью)
    int epochYear = static_cast<int>(parseTleDouble(l1, 18, 2)); // получаем две цифры года
    epochYear = (epochYear > 57) ? epochYear + 1900 : epochYear + 2000; // преобразуем к 4-м
    elements->epochdays = parseTleDouble(l1, 20, 12);

    // 6. Первая производная среднего движения - позиции 34-43
    elements->ndot = parseTleExpNumber(l1, 33, 10);

    // 7. Вторая производная среднего движения - позиции 45-52
    elements->nddot = parseTleExpNumber(l1, 44, 8);

    // 8. Коэффициент торможения B* - позиции 54-61
    elements->bstar = parseTleExpNumber(l1, 53, 8, true);

    // 9. Тип эфемерид - позиция 63 (0 = даны, 1 = SGP4/SDP4)
    elements->ephtype = static_cast<int>(parseTleDouble(l1, 62, 1));

    // 10. Номер элемента - позиции 65-68
    elements->elnum = static_cast<long>(parseTleDouble(l1, 64, 4));


    // === ПАРСИНГ ВТОРОЙ СТРОКИ TLE ===
    // 1. Номер строки (пропускаем)
    // 2. NORAD ID (должен совпадать с первой строкой) - пропускаем

    // 3. Наклонение (Inclination) - позиции 9-16

    elements->inclo = parseTleDouble(l2, 8, 8) * M_PI / 180.0;

    // 4. Прямое восхождение восходящего узла (RAAN) - позиции 18-25
    elements->nodeo = parseTleDouble(l2, 17, 8) * M_PI / 180.0;

    // 5. Эксцентриситет - позиции 27-33. В TLE он записан как 0.0000000, но без ведущей точки.
    // Например, '0000000' = 0.0, '1234567' = 0.1234567
    elements->ecco = parseTleDouble(l2, 26, 7) * 1e-7;

    // 6. Аргумент перигея - позиции 35-42
    elements->argpo = parseTleDouble(l2, 34, 8) * M_PI / 180.0; 

    // 7. Средняя аномалия - позиции 44-51
    elements->mo = parseTleDouble(l2, 43, 8) * M_PI / 180.0; 

    // 8. Среднее движение (оборотов в день) - позиции 53-63
    elements->no_kozai = parseTleDouble(l2, 52, 11) * (2 * M_PI) / (24.0 * 60.0); // радиан/мин;

    // 9. Номер витка (Revolution number) - позиции 64-68
    elements->revnum = static_cast<long>(parseTleDouble(l2, 63, 5));

    // === ВЫЧИСЛЕНИЕ ЮЛИАНСКОЙ ДАТЫ ЭПОХИ ===
    elements->epochyr = epochYear;
    elements->jdsatepoch = TimeManager::calcJulianDateFromEpoch(epochYear, elements->epochdays);

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
        std::cerr << "Ecco = " << tempRec.ecco << std::endl;
        std::cerr << "no_kozai = " << tempRec.no_kozai << std::endl;
        std::cout << "ndot=" << elements->ndot << ", nddot=" << elements->nddot
            << ", bstar=" << elements->bstar << std::endl;
        return false;
    }
}

int Sgp4Model::getNoradId() const
{
    return TleParser::extractNoradIdFromLine2(tleLine2);
}

double Sgp4Model::getEpochJulianDate() const
{
    try {
        return elements->jdsatepoch;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to getEpochJulianDate " << e.what() << std::endl;
        return 0.0;
    }
        
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
