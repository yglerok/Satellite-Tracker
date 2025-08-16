#include "Sun.h"
#include "Shaders.h"

#include <iostream>

glm::vec3 Sun::getDirection()
{
    double mjd = getJulianDate();
    std::cout << mjd << std::endl;

    // Вычисляем эклиптическую долготу Солнца
    double lambda = getEclipticLongitude(mjd);
    double beta = 0.0; // Широта Солнца близка к 0

    // Наклон эклиптики 
    double T = (mjd - 51544.5) / 36525.0;
    double epsilon = 23.4392911 - 0.0130042 * T - 1.64e-7 * T * T + 5.04e-7 * T * T * T;

    std::cout << "epsilon = " << epsilon << std::endl;

    double ra, dec;

    getEquatorialCoords(lambda, beta, epsilon, ra, dec);

    // Переводим RA из часов в радианы (15.0 * DEG_TO_RAD, т.к. 1 час = 15 градусов)
    double ra_rad = ra * 15.0 * DEG_TO_RAD;
    double dec_rad = dec * DEG_TO_RAD;

    glm::vec3 dir = glm::vec3(
        cos(dec_rad) * cos(ra_rad),
        cos(dec_rad) * sin(ra_rad),
        sin(dec_rad)
    );

    return glm::normalize(dir) * distance;
}

void Sun::setLightning(GLuint shaderProgram)
{
    glm::vec3 lightPos = getDirection();
    std::cout << "Sun direction: " << lightPos.x << ", " << lightPos.y << ", " << lightPos.z << std::endl;
    glUseProgram(shaderProgram);
    Shader::setVec3(shaderProgram, "lightPos", lightPos);
}

bool Sun::getLocalTime(const time_t* time, struct tm* result)
{
    if (!time || !result)
        return false;

    #if defined (_WIN32)
        return (localtime_s(result, time) == 0); // Windows
    #else
        // Linux/macOS (POSIX)
        std::tm* tmp = std::localtime(time);
        if (!tmp)
            return false;
        *result = *tmp;
        return true;
    #endif
}

std::tm Sun::getUtcTime(time_t time)
{
    std::tm tm_utc;

#if defined(_WIN32)
    gmtime_s(&tm_utc, &time); // Windows
#else
    tm_utc = *gmtime(&time); // Linux/macOS (POSIX)
#endif

    return tm_utc;
}

double Sun::getJulianDate()
{
    auto now = std::chrono::system_clock::now();
    time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm tm = getUtcTime(currentTime);

    int year = 1900 + tm.tm_year;
    int month = 1 + tm.tm_mon;
    int day = tm.tm_mday;
    int hour = tm.tm_hour; 
    int minute = tm.tm_min;
    int second = tm.tm_sec;

    std::cout << day << "." << month << "." << year <<
        " " << hour << ":" << minute << ":" << second << std::endl;

    if (month <= 2) { // в юлианском календаре год начинается с марта
        year--;
        month += 12;
    }
    int a = year / 100; // количество полных столетий с начала эпохи
    int b = 2 - a + a / 4; // поправка для перехода от юлианского к григорианскому календарю

    // 4716 — сдвиг эпохи (4716 год до н.э. = год 0 в астрономии)
    // 365.25 — средняя длина года с учётом високосных лет
    // 30.6001 — средняя длина месяца
    // month + 1 — коррекция для правильного учёта месяцев
    // 1524.5 — сдвиг для согласования с астрономической эпохой
    //double jd = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + b - 1524.5;
    //jd += (hour + minute / 60.0 + second / 3600.0) / 24.0;
    double jd = 367 * year - floor(7 * (year + floor((month + 9) / 12)) / 4)
        + floor(275 * month / 9) + day + 1721013.5
        + (hour + minute / 60.0 + second / 3600.0) / 24.0;
    
    return jd - 2400000.5; // перевод в MJD
}

double Sun::getEclipticLongitude(const double& mjd)
{
    // Вычисляем количество дней от эпохи J2000.0
    double T = (mjd - 51544.5) / 36525.0;

    // Средняя геометрическая долгота Солнца (градусы)
    double L0 = 280.46646 + 36000.76983 * T + 0.0003032 * T * T;
    L0 = fmod(L0, 360.0);
    if (L0 < 0) L0 += 360.0;

    // Средняя аномалия (градусы)
    double M_deg = 357.52911 + 35999.05029 * T - 0.0001537 * T * T;
    M_deg = fmod(M_deg, 360.0);
    if (M_deg < 0) 
        M_deg += 360.0;
    double M = M_deg * DEG_TO_RAD;

    // Уравнение центра
    double C = (1.914602 - 0.004817 * T - 0.000014 * T * T) * sin(M) +
        (0.019993 - 0.000101 * T) * sin(2 * M) +
        0.000289 * sin(3 * M);

    // Истинная долгота Солнца
    double true_longitude = L0 + C;
    true_longitude = fmod(true_longitude, 360.0);
    if (true_longitude < 0) true_longitude += 360.0;
    
    std::cout << "L0 = " << L0 << std::endl;
    std::cout << "M = " << M << std::endl;
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

    /*double y = sin(lambda) * cos(epsilon) - tan(beta) * sin(epsilon);*/
    double y = sin(lambda) * cos(epsilon);
    double x = cos(lambda);
    ra = atan2(y, x) * RAD_TO_DEG;
    if (ra < 0) ra += 360.0;
    ra /= 15.0; // Convert to hours

    std::cout << "ra = " << ra << std::endl;
    std::cout << "dec = " << dec << std::endl;

}
