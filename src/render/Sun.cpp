#include "Sun.h"
#include "Shaders.h"

#include <iostream>

glm::vec3 Sun::getDirection()
{
    double jd = timeManager->getCurrentJulianDate();
    std::cout << jd << std::endl;

    // Вычисляем эклиптическую долготу Солнца
    double lambda = getEclipticLongitude(jd);
    double beta = 0.0; // Широта Солнца близка к 0

    // Наклон эклиптики 
    double T = (jd - 2451545.0) / 36525.0;
    double epsilon = 23.4392911 - 0.0130042 * T - 1.64e-7 * T * T + 5.04e-7 * T * T * T;

    std::cout << "epsilon = " << epsilon << std::endl;

    double ra, dec;

    getEquatorialCoords(lambda, beta, epsilon, ra, dec);

    // ВЫЧИСЛЯЕМ ЧАСОВОЙ УГОЛ с учетом звездного времени
    double GMST = 280.46061837 + 360.98564736629 * (jd - 2451545.0)
        + 0.000387933 * T * T - T * T * T / 38710000.0;
    GMST = fmod(GMST, 360.0);
    if (GMST < 0) GMST += 360.0;

    double hourAngle = GMST - ra; // Часовой угол в градусах
    if (hourAngle < 0) hourAngle += 360.0;


    // Преобразуем в радианы
    double ha_rad = hourAngle * DEG_TO_RAD;
    double dec_rad = dec * DEG_TO_RAD;

    glm::vec3 dir = glm::vec3(
        cos(dec_rad) * cos(ha_rad),
        cos(dec_rad) * sin(ha_rad),
        sin(dec_rad) 
    );

    std::cout << "Normalized direction: " << glm::normalize(dir).x << ", "
        << glm::normalize(dir).y << ", " << glm::normalize(dir).z << std::endl;

    // Преобразуем в систему сцены (обычно Y-вверх):
    // X астро -> -Z сцены (или X сцены)
    // Y астро -> X сцены (или -Z сцены)  
    // Z астро -> Y сцены (вверх)

    glm::vec3 sceneDir = glm::vec3(
        dir.y,  // восток
        dir.z,  // вверх (северный полюс)
        -dir.x  // север (весеннее равноденствие -> юг)
    );

    std::cout << "=== SUN DEBUG ===" << std::endl;
    std::cout << "JD: " << jd << std::endl;
    std::cout << "RA: " << ra << "°, Dec: " << dec << "°" << std::endl;
    std::cout << "GMST: " << GMST << "°" << std::endl;
    std::cout << "Hour angle: " << hourAngle << "°" << std::endl;
    std::cout << "Local direction: " << dir.x << ", " << dir.y << ", " << dir.z << std::endl;

    std::cout << "OpenGL direction: " << sceneDir.x << ", " << sceneDir.y << ", " << sceneDir.z << std::endl;

    // Проверка положения Солнца
    if (hourAngle < 180.0) {
        std::cout << "Sun is on EASTERN hemisphere (rising)" << std::endl;
    }
    else {
        std::cout << "Sun is on WESTERN hemisphere (setting)" << std::endl;
    }

   /* std::cout << "sceneDir: " << glm::normalize(sceneDir).x << ", "
        << glm::normalize(sceneDir).y << ", " << glm::normalize(sceneDir).z << std::endl;*/

    // Инвертируем направление, потому что:
    // - Астрономические вычисления дают направление ИЗ центра Земли К Солнцу  
    // - Для освещения в OpenGL нужно направление ОТ Солнца К Земле
    return -glm::normalize(sceneDir);
}

Sun::Sun(std::shared_ptr<TimeManager> timeManager)
{
    this->timeManager = timeManager;
}

void Sun::setLightning(GLuint shaderProgram)
{
    glm::vec3 lightPos = getDirection();
    std::cout << "Sun direction: " << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << std::endl;
    glUseProgram(shaderProgram);
    Shader::setVec3(shaderProgram, "lightDir", lightPos);
}

double Sun::getEclipticLongitude(const double& jd)
{
    // Вычисляем количество дней от эпохи J2000.0
    double T = (jd - 2451545.0) / 36525.0;

    // Средняя геометрическая долгота Солнца (градусы)
    double L0 = 280.46646 + 36000.76983 * T + 0.0003032 * T * T;
    L0 = fmod(L0, 360.0);
    if (L0 < 0)
        L0 += 360.0;

    // Средняя аномалия (градусы)
    double M_deg = 357.52911 + 35999.05029 * T - 0.0001537 * T * T;
    M_deg = fmod(M_deg, 360.0);
    if (M_deg < 0) 
        M_deg += 360.0;
    double M_rad = M_deg * DEG_TO_RAD;

    // Уравнение центра
    double C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * sin(M_rad) +
        (0.019993 - 0.000101 * T) * sin(2 * M_rad) +
        0.000289 * sin(3 * M_rad);

    // Истинная долгота Солнца
    double true_longitude = L0 + C;
    true_longitude = fmod(true_longitude, 360.0);
    if (true_longitude < 0) 
        true_longitude += 360.0;
    
    std::cout << "L0 = " << L0 << std::endl;
    std::cout << "M = " << M_rad << std::endl;
    std::cout << "C = " << C << std::endl;
    std::cout << "true_longitude = " << true_longitude << std::endl;
    
    return true_longitude;

    //return fmod(L0 + C, 360.0);
}

void Sun::getEquatorialCoords(double lambda, double beta, double epsilon,
    double& ra, double& dec)
{
    lambda *= DEG_TO_RAD;
    beta *= DEG_TO_RAD;
    epsilon *= DEG_TO_RAD;

    double sin_dec = sin(beta) * cos(epsilon) + cos(beta) * sin(epsilon) * sin(lambda);
    dec = asin(sin_dec) * RAD_TO_DEG;

    double y = sin(lambda) * cos(epsilon) - tan(beta) * sin(epsilon);
    //double y = sin(lambda) * cos(epsilon);
    double x = cos(lambda);
    ra = atan2(y, x) * RAD_TO_DEG;
    if (ra < 0)
        ra += 360.0;

    double jd = timeManager->getCurrentJulianDate();
    double T = (jd - 2451545.0) / 36525.0;

    // Звездное время в Гринвиче (градусы)
    double GMST = 280.46061837 + 360.98564736629 * (jd - 2451545.0)
        + 0.000387933 * T * T - T * T * T / 38710000.0;
    GMST = fmod(GMST, 360.0);
    if (GMST < 0) GMST += 360.0;

    // Прямое восхождение с учетом времени суток
    double lst = GMST; // Для UTC+0
    double hourAngle = lst - ra;
    if (hourAngle < 0) hourAngle += 360.0;

    std::cout << "GMST: " << GMST << "°" << std::endl;
    std::cout << "Hour angle: " << hourAngle << "°" << std::endl;

    std::cout << "ra = " << ra << std::endl;
    std::cout << "dec = " << dec << std::endl;

}
