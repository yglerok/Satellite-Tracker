#include "TimeManager.h"

#include <iostream>
#include <iomanip>

TimeManager::TimeManager()
{
    currentTime = std::chrono::system_clock::now();
    calcJulianDate();
}

void TimeManager::update(double deltaTime)
{
    auto duration = std::chrono::duration_cast<std::chrono::system_clock::duration>(
        std::chrono::duration<double>(deltaTime)
    );
    currentTime += duration;
}

time_t TimeManager::toTimeT(std::chrono::system_clock::time_point timePoint)
{
    return std::chrono::system_clock::to_time_t(timePoint);
}

std::string TimeManager::getStringCurrentTime()
{
    std::stringstream ss;
    time_t tt = toTimeT(currentTime);
    ss << std::put_time(std::localtime(&tt), "%F %T") << std::endl;
    std::string str = ss.str();
    return str;
}

void TimeManager::calcJulianDate()
{
    std::tm tm = getUtcTime(toTimeT(currentTime));

    int year = 1900 + tm.tm_year;
    int month = 1 + tm.tm_mon;
    int day = tm.tm_mday;
    int hour = tm.tm_hour;
    int minute = tm.tm_min;
    int second = tm.tm_sec;

    std::cout << day << "." << month << "." << year <<
        " " << hour << ":" << minute << ":" << second << std::endl;

    if (month <= 2) { // в юлианском календаре год начинаетс€ с марта
        year--;
        month += 12;
    }
    int a = year / 100; // количество полных столетий с начала эпохи
    int b = 2 - a + a / 4; // поправка дл€ перехода от юлианского к григорианскому календарю

    // 4716 Ч сдвиг эпохи (4716 год до н.э. = год 0 в астрономии)
    // 365.25 Ч средн€€ длина года с учЄтом високосных лет
    // 30.6001 Ч средн€€ длина мес€ца
    // month + 1 Ч коррекци€ дл€ правильного учЄта мес€цев
    // 1524.5 Ч сдвиг дл€ согласовани€ с астрономической эпохой
    //double jd = floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1)) + day + b - 1524.5;
    //jd += (hour + minute / 60.0 + second / 3600.0) / 24.0;
    julianDate = 367 * year - floor(7 * (year + floor((month + 9) / 12)) / 4)
        + floor(275 * month / 9) + day + 1721013.5
        + (hour + minute / 60.0 + second / 3600.0) / 24.0;
}

std::tm TimeManager::getUtcTime(time_t time)
{
    std::tm tm_utc;

#if defined(_WIN32)
    gmtime_s(&tm_utc, &time); // Windows
#else
    tm_utc = *gmtime(&time); // Linux/macOS (POSIX)
#endif

    return tm_utc;
}
