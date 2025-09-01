#pragma once

#include <ctime>
#include <chrono>
#include <string>

class TimeManager
{
public:
	TimeManager();

	void update(double deltaTime);

	double getCurrentJulianDate() const { return julianDate; }
	std::chrono::system_clock::time_point getCurrentTime() const { return currentTime; }
	time_t toTimeT(std::chrono::system_clock::time_point timePoint);
	std::string getStringCurrentTime();

	static double calcJulianDateFromEpoch(int epochYear, double epochDay);

private:
	double julianDate;
	std::chrono::system_clock::time_point currentTime;

	static double calcJulianDate(int year, int month, int day, int hour, int minute, int second);
	static void calcMonthDayFromEpoch(int year, double dayOfYear, int& month, int& day);
	void calCurrentJulianDate();
	std::tm getUtcTime(time_t time);
};

//bool getLocalTime(const time_t* time, struct tm* result)
//{
//    if (!time || !result)
//        return false;
//
//#if defined (_WIN32)
//    return (localtime_s(result, time) == 0); // Windows
//#else
//    // Linux/macOS (POSIX)
//    std::tm* tmp = std::localtime(time);
//    if (!tmp)
//        return false;
//    *result = *tmp;
//    return true;
//#endif
//}