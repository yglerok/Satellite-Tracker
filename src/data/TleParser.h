#pragma once

#include <string>
#include <vector>
#include <optional>

#include "Database.h"

class TleParser
{
public:
	TleParser() = default;
	~TleParser() = default;

	std::vector<SatelliteTle> parseTleData(const std::string& data);
	std::vector<SatelliteTle> parseTleFile(const std::string& fileName);

	bool validateTleLine(const std::string& line, int lineNumber);
	bool validateTleBlock(const std::string& line0, const std::string& line1, const std::string& line2);

	int extractNoradIdFromLine2(const std::string& line2);
	std::string extractEpochFromLine1(const std::string& line1);
	std::string extractNameFromLine0(const std::string& line0);

private:
	std::string cleanTleLine(const std::string& line);
	bool isTleLineValid(int lineNumber, const std::string& line);

	//bool validateChecksum(const std::string& line);
	//int calculateChecksum(const std::string& line);
};