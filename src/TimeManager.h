#pragma once

#include <ctime>
#include <chrono>
#include <string>

class TimeManager
{
public:
	TimeManager();

	void update(double deltaTime);

	double getJulianDate() const { return julianDate; }
	std::chrono::system_clock::time_point getCurrentTime() const { return currentTime; }
	time_t toTimeT(std::chrono::system_clock::time_point timePoint);
	std::string getStringCurrentTime();

private:
	double julianDate;
	std::chrono::system_clock::time_point currentTime;

	void calcJulianDate();
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