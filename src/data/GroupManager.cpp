#include "GroupManager.h"
#include <algorithm>
#include <iostream>

GroupManager::GroupManager(std::shared_ptr<Database> database)
{
	this->database = database;
}

bool GroupManager::initialize()
{
    // Создаем стандартные группы если их нет
    const std::vector<std::string> defaultGroups = {
        "Starlink",
        "GPS",
        "GLONASS",
        "Galileo",
        "BeiDou",
        "Weather",
        "ISS",
        "Space Stations",
        "Scientific",
        "Geostationary",
        "Low Earth Orbit",
        "Medium Earth Orbit",
        "High Earth Orbit",
        "Other"
    };

    for (const auto& group : defaultGroups) {
        if (!database->getSatellitesByGroup(group).empty() || createGroup(group)) {
            SatelliteGroup sg;
            sg.name = group;
            sg.isVisible = true;
            groups[group] = sg;
        }
    }

    // Загружаем существующие группы из базы
    auto groupNames = database->getAllGroups();
    for (const auto& groupName : groupNames) {
        if (groups.find(groupName) == groups.end()) {
            SatelliteGroup sg;
            sg.name = groupName;
            sg.isVisible = true;
            groups[groupName] = sg;
        }
    }

    return true;
}

bool GroupManager::createGroup(const std::string& groupName)
{
    if (groups.find(groupName) != groups.end())
        return false;

    SatelliteGroup sg;
    sg.name = groupName;
    sg.isVisible = true;
    groups[groupName] = sg;

    return true;
}

bool GroupManager::deleteGroup(const std::string& groupName)
{
    auto it = groups.find(groupName);
    if (it != groups.end()) {
        groups.erase(it);
        return database->deleteGroup(groupName);
    }
    return false;
}

bool GroupManager::addSatelliteToGroup(int noradId, const std::string& groupName)
{
    if (groups.find(groupName) == groups.end()) {
        if (!createGroup(groupName))
            return false;
    }

    groups[groupName].noradIds.push_back(noradId);

    return database->addSatelliteToGroup(noradId, groupName);
}

bool GroupManager::sortSatellitesIntoGroups()
{
    auto allSatellites = database->getAllSatellites();
    std::cout << "Sorting " << allSatellites.size() << " satellites into groups..." << std::endl;

    // Очищаем текущие группы
    for (auto& [groupName, group] : groups) {
        group.noradIds.clear();
    }

    int categorizedCount = 0;

    
    for (const auto& satellite : allSatellites) {
        bool categorized = false;

        // Определяем группу на основе характеристик спутника
        if (isStarlinkSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Starlink");
            categorized = true;
        }
        else if (isGNSSSatellite(satellite)) {
            if (satellite.name.find("GPS") != std::string::npos) {
                addSatelliteToGroup(satellite.noradId, "GPS");
            }
            else if (satellite.name.find("GLONASS") != std::string::npos) {
                addSatelliteToGroup(satellite.noradId, "GLONASS");
            }
            else if (satellite.name.find("NAVSTAR") != std::string::npos) {
                addSatelliteToGroup(satellite.noradId, "NAVSTAR");
            }
            else if (satellite.name.find("GALILEO") != std::string::npos ||
                satellite.name.find("Galileo") != std::string::npos) {
                addSatelliteToGroup(satellite.noradId, "Galileo");
            }
            else if (satellite.name.find("BEIDOU") != std::string::npos ||
                satellite.name.find("BeiDou") != std::string::npos) {
                addSatelliteToGroup(satellite.noradId, "BeiDou");
            }
            categorized = true;
        }
        else if (isWeatherSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Weather");
            categorized = true;
        }
        else if (isSpaceStation(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Space Stations");
            categorized = true;
        }
        else if (isScientificSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Scientific");
            categorized = true;
        }

        //categorizeByOrbit(satellite);

        if (categorized) 
            categorizedCount++;    
    }
    
    std::cout << "Categorized " << categorizedCount << " out of "
            << allSatellites.size() << " satellites" << std::endl;

        return true;
}

std::vector<std::string> GroupManager::getGroupNames() const
{
    return database->getAllGroups();
}

std::vector<SatelliteTle> GroupManager::getSatellitesInGroup(const std::string& groupName) const
{
    return database->getSatellitesByGroup(groupName);
}

void GroupManager::setGroupVisibility(const std::string& groupName, bool visible)
{
    auto it = groups.find(groupName);
    if (it != groups.end()) {
        it->second.isVisible = visible;
    }
}

bool GroupManager::isGroupVisible(const std::string& groupName) const
{
    auto it = groups.find(groupName);
    if (it != groups.end()) {
        return it->second.isVisible;
    }
    return false;
}

bool GroupManager::isStarlinkSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("STARLINK") != std::string::npos ||
        satellite.name.find("Starlink") != std::string::npos ||
        satellite.tleLine2.substr(0, 5) == "STARL";
}

bool GroupManager::isWeatherSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("NOAA") != std::string::npos ||
        satellite.name.find("METEOR") != std::string::npos ||
        satellite.name.find("GOES") != std::string::npos ||
        satellite.name.find("METOP") != std::string::npos ||
        satellite.name.find("FY") != std::string::npos;
}

bool GroupManager::isGNSSSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("GPS") != std::string::npos ||
        satellite.name.find("GLONASS") != std::string::npos ||
        satellite.name.find("GALILEO") != std::string::npos ||
        satellite.name.find("BEIDOU") != std::string::npos ||
        satellite.name.find("NAVSTAR") != std::string::npos;
}

bool GroupManager::isSpaceStation(const SatelliteTle& satellite) const
{
    return satellite.name.find("ISS") != std::string::npos ||
        satellite.name.find("TIANGONG") != std::string::npos ||
        satellite.name.find("Tiangong") != std::string::npos;
}

bool GroupManager::isScientificSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("HUBBLE") != std::string::npos ||
        satellite.name.find("JAMES WEBB") != std::string::npos ||
        satellite.name.find("TESS") != std::string::npos ||
        satellite.name.find("KEPLER") != std::string::npos ||
        satellite.name.find("HST") != std::string::npos;
}

bool GroupManager::isISS(const SatelliteTle& satellite) const
{
    return satellite.name.find("ISS") != std::string::npos;
}

void GroupManager::categorizeByOrbit(const SatelliteTle& satellite)
{
    // Анализируем вторую строку TLE для определения орбиты
    // Пример: "2 25544  51.6440  48.0927 0005526 297.3889 249.1842 15.49807819372857"
    // Элементы: eccentricity (6), inclination (8), mean motion (13)

    try {
        double inclination = std::stod(satellite.tleLine2.substr(8, 8));
        double eccentricity = std::stod(satellite.tleLine2.substr(26, 7)) / 10000000.0;
        double meanMotion = std::stod(satellite.tleLine2.substr(52, 11));

        if (std::abs(inclination - 0.0) < 1.0 && eccentricity < 0.01) {
            addSatelliteToGroup(satellite.noradId, "Geostationary");
        }
        else if (meanMotion > 11.25 && inclination < 55.0) {
            addSatelliteToGroup(satellite.noradId, "Low Earth Orbit");
        }
        else if (meanMotion > 2.0 && meanMotion <= 11.25) {
            addSatelliteToGroup(satellite.noradId, "Medium Earth Orbit");
        }
        else {
            addSatelliteToGroup(satellite.noradId, "High Earth Orbit");
        }
    }
    catch (...) {
        // В случае ошибки парсинга добавляем в Other
        addSatelliteToGroup(satellite.noradId, "Other");
    }
}
