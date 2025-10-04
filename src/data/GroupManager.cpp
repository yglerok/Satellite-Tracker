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
        "Geosynchronous Orbit",
        "Low Earth Orbit",
        "Medium Earth Orbit",
        "High Earth Orbit",
        "Highly Elliptical Orbit",
        "Polar Orbit",
        "Very Low Earth Orbit",
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

        // === 1. КРУПНЫЕ СПУТНИКОВЫЕ СОЗВЕЗДИЯ ===
        if (isStarlinkSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Starlink");
            categorized = true;
        }
        else if (isOneWebSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "OneWeb");
            categorized = true;
        }
        else if (isIridiumSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Iridium");
            categorized = true;
        }
        else if (isGlobalstarSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Globalstar");
            categorized = true;
        }
        else if (isOrbcommSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "ORBCOMM");
            categorized = true;
        }
        else if (isPlanetLabsSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Planet Labs");
            categorized = true;
        }
        else if (isSpireSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Spire");
            categorized = true;
        }

        // === 2. НАВИГАЦИОННЫЕ СИСТЕМЫ ===
        else if (isGPSSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "GPS");
            categorized = true;
        }
        else if (isGLONASSSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "GLONASS");
            categorized = true;
        }
        else if (isGalileoSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Galileo");
            categorized = true;
        }
        else if (isBeiDouSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "BeiDou");
            categorized = true;
        }
        else if (isIRNSSSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "IRNSS");
            categorized = true;
        }
        else if (isQZSSSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "QZSS");
            categorized = true;
        }

        // === 3. КОММУНИКАЦИОННЫЕ ОПЕРАТОРЫ ===
        else if (isIntelsatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Intelsat");
            categorized = true;
        }
        else if (isSESsatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "SES");
            categorized = true;
        }
        else if (isEutelsatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Eutelsat");
            categorized = true;
        }
        else if (isTelesatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Telesat");
            categorized = true;
        }
        else if (isInmarsatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Inmarsat");
            categorized = true;
        }
        else if (isThurayaSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Thuraya");
            categorized = true;
        }

        // === 4. НАУЧНЫЕ И ИССЛЕДОВАТЕЛЬСКИЕ ===
        else if (isNASASatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "NASA");
            categorized = true;
        }
        else if (isESASatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "ESA");
            categorized = true;
        }
        else if (isHubbleTelescope(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Hubble Space Telescope");
            categorized = true;
        }
        else if (isJamesWebbTelescope(satellite)) {
            addSatelliteToGroup(satellite.noradId, "James Webb Telescope");
            categorized = true;
        }
        else if (isPlanetaryScienceSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Planetary Science");
            categorized = true;
        }
        else if (isAstronomySatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Astronomy");
            categorized = true;
        }

        // === 5. МЕТЕОРОЛОГИЧЕСКИЕ ===
        else if (isNOAASatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "NOAA");
            categorized = true;
        }
        else if (isGOESSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "GOES");
            categorized = true;
        }
        else if (isMeteosatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Meteosat");
            categorized = true;
        }
        else if (isFengyunSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Fengyun");
            categorized = true;
        }
        else if (isElectroLSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Electro-L");
            categorized = true;
        }

        // === 6. ВОЕННЫЕ И РАЗВЕДЫВАТЕЛЬНЫЕ ===
        else if (isUSMilitarySatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "US Military");
            categorized = true;
        }
        else if (isRussianMilitarySatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Russian Military");
            categorized = true;
        }
        else if (isReconnaissanceSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Reconnaissance");
            categorized = true;
        }
        else if (isEarlyWarningSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Early Warning");
            categorized = true;
        }

        // === 7. ЗЕМЛЕНАБЛЮДЕНИЕ ===
        else if (isLandsatSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Landsat");
            categorized = true;
        }
        else if (isSentinelSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Sentinel");
            categorized = true;
        }
        else if (isSpotSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "SPOT");
            categorized = true;
        }
        else if (isHighResolutionImagingSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "High Resolution Imaging");
            categorized = true;
        }

        // === 8. СПЕЦИАЛИЗИРОВАННЫЕ ГРУППЫ ===
        else if (isAmateurRadioSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Amateur Radio");
            categorized = true;
        }
        else if (isCubeSat(satellite)) {
            addSatelliteToGroup(satellite.noradId, "CubeSat");
            categorized = true;
        }
        else if (isTechnologyDemoSatellite(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Technology Demo");
            categorized = true;
        }
        else if (isSpaceStationRelated(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Space Station Related");
            categorized = true;
        }
        else if (isDebrisOrRocketBody(satellite)) {
            addSatelliteToGroup(satellite.noradId, "Debris/Rocket Body");
            categorized = true;
        }

        // === 9. ОРБИТАЛЬНАЯ КЛАССИФИКАЦИЯ ===
        categorizeByOrbit(satellite);

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

bool GroupManager::isGlobalstarSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("GLOBALSTAR") != std::string::npos;
}

bool GroupManager::isOrbcommSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("ORBCOMM") != std::string::npos;
}

bool GroupManager::isPlanetLabsSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("FLOCK") != std::string::npos ||
        satellite.name.find("DOVE") != std::string::npos ||
        satellite.name.find("SKYSAT") != std::string::npos;
}

bool GroupManager::isSpireSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("LEMUR") != std::string::npos ||
        satellite.name.find("SPIRE") != std::string::npos;
}

// 2. Навигационные системы (детализация)
bool GroupManager::isGPSSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("GPS") != std::string::npos ||
        satellite.name.find("NAVSTAR") != std::string::npos;
}

bool GroupManager::isGLONASSSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("GLONASS") != std::string::npos;
}

bool GroupManager::isGalileoSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("GALILEO") != std::string::npos;
}

bool GroupManager::isBeiDouSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("BEIDOU") != std::string::npos;
}

bool GroupManager::isIRNSSSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("IRNSS") != std::string::npos;
}

bool GroupManager::isQZSSSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("QZSS") != std::string::npos;
}

// 3. Коммуникационные операторы
bool GroupManager::isTelesatSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("TELESAT") != std::string::npos;
}

bool GroupManager::isInmarsatSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("INMARSAT") != std::string::npos;
}

bool GroupManager::isThurayaSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("THURAYA") != std::string::npos;
}

// 4. Научные и исследовательские
bool GroupManager::isNASASatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("NASA") != std::string::npos ||
        satellite.name.find("Explorer") != std::string::npos;
}

bool GroupManager::isESASatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("ESA") != std::string::npos;
}

bool GroupManager::isHubbleTelescope(const SatelliteTle& satellite) const {
    return satellite.name.find("HUBBLE") != std::string::npos ||
        satellite.name.find("HST") != std::string::npos;
}

bool GroupManager::isJamesWebbTelescope(const SatelliteTle& satellite) const {
    return satellite.name.find("JAMES WEBB") != std::string::npos ||
        satellite.name.find("JWST") != std::string::npos;
}

bool GroupManager::isPlanetaryScienceSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("VOYAGER") != std::string::npos ||
        satellite.name.find("CASSINI") != std::string::npos ||
        satellite.name.find("MARS") != std::string::npos ||
        satellite.name.find("LUNAR") != std::string::npos;
}

bool GroupManager::isAstronomySatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("TESS") != std::string::npos ||
        satellite.name.find("KEPLER") != std::string::npos ||
        satellite.name.find("CHANDRA") != std::string::npos ||
        satellite.name.find("SWIFT") != std::string::npos;
}

// 5. Метеорологические
bool GroupManager::isNOAASatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("NOAA") != std::string::npos;
}

bool GroupManager::isGOESSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("GOES") != std::string::npos;
}

bool GroupManager::isMeteosatSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("METEOSAT") != std::string::npos;
}

bool GroupManager::isFengyunSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("FY-") != std::string::npos ||
        satellite.name.find("FENGYUN") != std::string::npos;
}

bool GroupManager::isElectroLSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("ELEKTRO") != std::string::npos;
}

// 6. Военные и разведывательные
bool GroupManager::isUSMilitarySatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("USA") != std::string::npos ||
        satellite.name.find("MILSTAR") != std::string::npos ||
        satellite.name.find("DSP") != std::string::npos;
}

bool GroupManager::isRussianMilitarySatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("COSMOS") != std::string::npos;
}

bool GroupManager::isReconnaissanceSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("KEYHOLE") != std::string::npos ||
        satellite.name.find("LACROSSE") != std::string::npos;
}

bool GroupManager::isEarlyWarningSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("SBIRS") != std::string::npos ||
        satellite.name.find("DSP") != std::string::npos;
}

// 7. Мониторинг Земли
bool GroupManager::isLandsatSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("LANDSAT") != std::string::npos;
}

bool GroupManager::isSentinelSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("SENTINEL") != std::string::npos;
}

bool GroupManager::isSpotSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("SPOT") != std::string::npos;
}

bool GroupManager::isHighResolutionImagingSatellite(const SatelliteTle& satellite) const {
    return satellite.name.find("WORLDVIEW") != std::string::npos ||
        satellite.name.find("GEOEYE") != std::string::npos ||
        satellite.name.find("QUICKBIRD") != std::string::npos;
}

// 8. Специализированные группы
bool GroupManager::isSpaceStationRelated(const SatelliteTle& satellite) const {
    return satellite.name.find("ISS") != std::string::npos ||
        satellite.name.find("TIANGONG") != std::string::npos ||
        satellite.name.find("PROGRESS") != std::string::npos ||
        satellite.name.find("SOYUZ") != std::string::npos;
}

bool GroupManager::isDebrisOrRocketBody(const SatelliteTle& satellite) const {
    return satellite.name.find("DEB") != std::string::npos ||
        satellite.name.find("R/B") != std::string::npos ||
        satellite.name.find("DEBRIS") != std::string::npos;
}

bool GroupManager::isOneWebSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("ONEWEB") != std::string::npos ||
        satellite.name.find("OneWeb") != std::string::npos;
}

bool GroupManager::isIridiumSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("IRIDIUM") != std::string::npos ||
        satellite.name.find("Iridium") != std::string::npos;
}

bool GroupManager::isIntelsatSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("INTELSAT") != std::string::npos ||
        satellite.name.find("Intelsat") != std::string::npos ||
        satellite.name.find("IS-") != std::string::npos;
}

bool GroupManager::isSESsatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("SES") != std::string::npos ||
        satellite.name.find("NSS-") != std::string::npos;
}

bool GroupManager::isEutelsatSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("EUTELSAT") != std::string::npos ||
        satellite.name.find("Eutelsat") != std::string::npos ||
        satellite.name.find("EUTELSAT") != std::string::npos;
}

bool GroupManager::isAmateurRadioSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("AMSAT") != std::string::npos ||
        satellite.name.find("OSCAR") != std::string::npos ||
        satellite.name.find("AO-") != std::string::npos ||
        satellite.name.find("FO-") != std::string::npos ||
        satellite.name.find("PO-") != std::string::npos ||
        satellite.name.find("RS-") != std::string::npos ||
        satellite.name.find("CAS-") != std::string::npos;
}

bool GroupManager::isCubeSat(const SatelliteTle& satellite) const
{
    return satellite.name.find("CUBESAT") != std::string::npos ||
        satellite.name.find("CubeSat") != std::string::npos ||
        satellite.name.find("QB50") != std::string::npos ||
        satellite.name.find("1U") != std::string::npos ||
        satellite.name.find("2U") != std::string::npos ||
        satellite.name.find("3U") != std::string::npos;
}

bool GroupManager::isTechnologyDemoSatellite(const SatelliteTle& satellite) const
{
    return satellite.name.find("TECHNOLOGY") != std::string::npos ||
        satellite.name.find("TECH DEMO") != std::string::npos ||
        satellite.name.find("DEMONSTRATION") != std::string::npos ||
        satellite.name.find("EXPERIMENT") != std::string::npos ||
        satellite.name.find("TEST") != std::string::npos;
}

//bool GroupManager::isWeatherSatellite(const SatelliteTle& satellite) const
//{
//    return satellite.name.find("NOAA") != std::string::npos ||
//        satellite.name.find("METEOR") != std::string::npos ||
//        satellite.name.find("GOES") != std::string::npos ||
//        satellite.name.find("METOP") != std::string::npos ||
//        satellite.name.find("FY") != std::string::npos;
//}
//
//bool GroupManager::isGNSSSatellite(const SatelliteTle& satellite) const
//{
//    return satellite.name.find("GPS") != std::string::npos ||
//        satellite.name.find("GLONASS") != std::string::npos ||
//        satellite.name.find("GALILEO") != std::string::npos ||
//        satellite.name.find("BEIDOU") != std::string::npos ||
//        satellite.name.find("NAVSTAR") != std::string::npos;
//}
//
//bool GroupManager::isSpaceStation(const SatelliteTle& satellite) const
//{
//    return satellite.name.find("ISS") != std::string::npos ||
//        satellite.name.find("TIANGONG") != std::string::npos ||
//        satellite.name.find("Tiangong") != std::string::npos;
//}
//
//bool GroupManager::isScientificSatellite(const SatelliteTle& satellite) const
//{
//    return satellite.name.find("HUBBLE") != std::string::npos ||
//        satellite.name.find("JAMES WEBB") != std::string::npos ||
//        satellite.name.find("TESS") != std::string::npos ||
//        satellite.name.find("KEPLER") != std::string::npos ||
//        satellite.name.find("HST") != std::string::npos;
//}
//
//bool GroupManager::isISS(const SatelliteTle& satellite) const
//{
//    return satellite.name.find("ISS") != std::string::npos;
//}

void GroupManager::categorizeByOrbit(const SatelliteTle& satellite)
{
    // Анализируем вторую строку TLE для определения орбиты
    // Пример: "2 25544  51.6440  48.0927 0005526 297.3889 249.1842 15.49807819372857"
    // Элементы: eccentricity (6), inclination (8), mean motion (13)

    try {
        double inclination = std::stod(satellite.tleLine2.substr(8, 8));
        double eccentricity = std::stod(satellite.tleLine2.substr(26, 7)) / 10000000.0;
        double meanMotion = std::stod(satellite.tleLine2.substr(52, 11));
    
        // Вычисляем период обращения в минутах
        double periodMinutes = 1440.0 / meanMotion;
    
        // Вычисляем высоту орбиты (приблизительно для круговых орбит)
        const double earthRadius = 6371.0; 
        const double mu = 398600.4418;
        double periodSeconds = periodMinutes * 60.0;
        double semiMajorAxis = pow((mu * periodSeconds * periodSeconds) / (4 * M_PI * M_PI), 1.0 / 3.0);
        double altitude = semiMajorAxis - earthRadius;
    
        // Классификация орбит
        if (eccentricity > 0.1) {
            addSatelliteToGroup(satellite.noradId, "Highly Elliptical Orbit");
        }
        // Геостационарные орбиты (GEO)
        else if (std::abs(periodMinutes - 1436.0) < 10.0) { // Период ~1 звездные сутки
            if (inclination < 1.0 && eccentricity < 0.001) {
                // Идеальная геостационарная орбита
                addSatelliteToGroup(satellite.noradId, "Geostationary");
            }
            else {
                // Геосинхронная но не геостационарная
                addSatelliteToGroup(satellite.noradId, "Geosynchronous Orbit");
            }
        }
        // Средние орбиты (MEO)
        else if (altitude > 2000 && altitude <= 35786) {
            addSatelliteToGroup(satellite.noradId, "Medium Earth Orbit");
        }
        // Низкие орбиты (LEO)
        else if (altitude <= 2000) {
            if (inclination > 80.0 && inclination < 100.0) {
                addSatelliteToGroup(satellite.noradId, "Polar Orbit");
            }
            else if (altitude < 600) {
                addSatelliteToGroup(satellite.noradId, "Very Low Earth Orbit");
            }
            else {
                addSatelliteToGroup(satellite.noradId, "Low Earth Orbit");
            }
        }
        // Высокие орбиты (HEO) - выше геостационарной
        else {
            addSatelliteToGroup(satellite.noradId, "High Earth Orbit");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error categorizing orbit for satellite " << satellite.noradId
            << ": " << e.what() << std::endl;
        addSatelliteToGroup(satellite.noradId, "Other");
    }

}
