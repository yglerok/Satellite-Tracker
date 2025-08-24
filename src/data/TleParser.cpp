#include "TleParser.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <regex>

std::vector<SatelliteTle> TleParser::parseTleData(const std::string& data)
{
    std::vector<SatelliteTle> satellites;

    if (data.empty()) {
        std::cerr << "Empty TLE data provided!" << std::endl;
        return satellites;
    }

    std::istringstream stream(data);
    std::string line0, line1, line2;
    int lineCount = 0;

    while (std::getline(stream, line0)) {
        lineCount++;
        if (line0.empty() || line0[0] == '#')
            continue;

        // „итаем следующие две строки после названи€
        if (!std::getline(stream, line1) || !std::getline(stream, line2))
            break;

        lineCount += 2;

        line0 = cleanTleLine(line0);
        line1 = cleanTleLine(line1);
        line2 = cleanTleLine(line2);

        // ¬алидаци€ TLE блока
        if (!validateTleBlock(line0, line1, line2)) {
            std::cerr << "Invalid TLE block at line " << (lineCount - 2) << std::endl;
            continue;
        }

        int noradId = extractNoradIdFromLine2(line2);
        if (noradId == -1) {
            std::cerr << "Failed to extract NORAD ID for: " << line0 << std::endl;
            continue;
        }

        SatelliteTle satellite;
        satellite.name = extractNameFromLine0(line0);
        satellite.noradId = noradId;
        satellite.tleLine1 = line1;
        satellite.tleLine2 = line2;
        satellite.epoch = extractEpochFromLine1(line1);

        satellites.push_back(satellite);
    }

    std::cout << "Parsed " << satellites.size() << " satellites from TLE data" << std::endl;
    return satellites;
}

std::vector<SatelliteTle> TleParser::parseTleFile(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return {};
    }

    std::string content;
    file.seekg(0, std::ios::end); // переводим указатель в конец файла
    content.reserve(file.tellg()); // с помощью tellg получаем текущую позицию (т.е. размер)
                                   // и резервируем у строки
    file.seekg(0, std::ios::beg); // возвращем указатель в начало
    content.assign(std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());  // замен€ем данные в строке

    file.close();
    return parseTleData(content);
}

bool TleParser::validateTleLine(const std::string& line, int lineNumber)
{
    if (line.empty()) {
        std::cerr << "Line " << lineNumber << " is empty" << std::endl;
        return false;
    }

    // ѕровер€ем длину строки (стандартна€ длина - 69 символов)
    if (line.length() < 68) {
        std::cerr << "Line " << lineNumber << " is too short: " << line.length() 
            << " characters" << std::endl;
        return false;
    }

    return true;
}

bool TleParser::validateTleBlock(const std::string& line0, const std::string& line1,
    const std::string& line2)
{
    if (line1.empty() || line1[0] != '1') {
        std::cerr << "Line 1 should start with '1': " << line1 << std::endl;
        return false;
    }

    if (line2.empty() || line2[0] != '2') {
        std::cerr << "Line 2 should start with '2': " << line2 << std::endl;
        return false;
    }

    if (!validateTleLine(line1, 1) || !validateTleLine(line2, 2))
        return false;

    return true;
}

int TleParser::extractNoradIdFromLine2(const std::string& line2)
{
    try {
        // NORAD ID находитс€ в позици€х 2-6 во второй строке
        // ‘ормат: "2 25544   ..."
        if (line2.length() < 7) {
            std::cerr << "Line 2 too short for NORAD ID extraction" << std::endl;
            return -1;
        }
        
        std::string idStr = line2.substr(2, 5);
        idStr.erase(std::remove(idStr.begin(), idStr.end(), ' '), idStr.end());

        if (idStr.empty()) {
            std::cerr << "Empty NORAD ID string" << std::endl;
            return -1;
        }
            
        for (char c : idStr) {
            if (!isdigit(c)) {
                std::cerr << "Invalid character in NORAD ID: " << idStr << std::endl;
                return -1;
            }
        }

        return std::atoi(idStr.c_str());
    }
    catch (const std::exception& e) {
        std::cerr << "Error extracting NORAD ID: " << e.what() << std::endl;
        return -1;
    }
}

std::string TleParser::extractEpochFromLine1(const std::string& line1)
{
    if (line1.length() < 32)
        return "unknown";
    return line1.substr(18, 14);
}

std::string TleParser::extractNameFromLine0(const std::string& line0)
{
    return cleanTleLine(line0);
}

std::string TleParser::cleanTleLine(const std::string& line)
{
    std::string cleaned = line;

    // убираем символы возврата коретки и переноса строки
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '\r'), cleaned.end());
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '\n'), cleaned.end());

    // убираем лишние пробелы в начале и в конце
    size_t start = cleaned.find_first_not_of(" \t");
    size_t end = cleaned.find_last_not_of(" \t");

    if (start != std::string::npos && end != std::string::npos)
        cleaned = cleaned.substr(start, end - start + 1);

    return cleaned;
}

