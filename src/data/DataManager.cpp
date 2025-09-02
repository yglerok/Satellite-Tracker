#include "DataManager.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <filesystem>

DataManager::DataManager(const std::string& dbPath)
{
	database = std::make_shared<Database>(dbPath);
	groupManager = std::make_unique<GroupManager>(database);
}

DataManager::~DataManager()
{
	curl_global_cleanup();
}

bool DataManager::initialize()
{
	if (!database->isOpen()) {
		std::cerr << "Failed to init DataManager! Database is not open!" << std::endl;
		return false;
	}

	if (!database->createTables()) {
		std::cerr << "Failed to create database tables" << std::endl;
		return false;
	}

	if (!groupManager->initialize()) {
		std::cerr << "Failed to initialize GroupManager" << std::endl;
		return false;
	}

	return true;
}

DataManager::UpdateResult DataManager::updateTleData(const std::string& backupFilePath, const std::chrono::hours& updateInterval)
{
	// Проверяем, когда было предыдущее обновление
	auto lastUpdate = getLastTleUpdateTime();
	auto now = std::chrono::system_clock::now();

	if (lastUpdate.has_value()) {
		auto timeSienceLastUpdate = now - lastUpdate.value();
		if (timeSienceLastUpdate < updateInterval) {
			auto hours = std::chrono::duration_cast<std::chrono::hours>(timeSienceLastUpdate).count();
			std::cout << "Using cached TLE data (updated " << hours << " hours ago)" << std::endl;
			return UpdateResult::UsingCachedData;
		}
	}

	std::cout << "Attempting to download fresh TLE data from Celestrack..." << std::endl;

	auto networkData = downloadTleData();
	if (networkData.has_value() && !networkData->empty()) {
		std::cout << "Successfully downloaded TLE data from network (" << networkData->size() << " bytes)" << std::endl;

		// Сохраняем в файл
		if (saveTleDataToFile(networkData.value(), backupFilePath))
			std::cout << "Backup saved to: " << backupFilePath << std::endl;
		else
			std::cerr << "Warning: Failed to save backup to " << backupFilePath << std::endl;

		// Обновляем бд
		if (updateTleFromString(networkData.value())) {
			std::cout << "Database updated successfully from network data" << std::endl;
			return UpdateResult::SuccessFromNetwork;
		} else
			std::cerr << "Failed to update database from network data" << std::endl;
	} else
		std::cout << "Network download failed or returned empty data" << std::endl;

	// Если загрузка из сети недоступна или не удалась - загружаем из файла
	std :: cout << "Trying to load from backup file: " << backupFilePath << std::endl;

	if (std::filesystem::exists(backupFilePath)) {
		if (updateTleFromFile(backupFilePath)) {
			std::cout << "Successfully loaded TLE data from backup file" << std::endl;
			return UpdateResult::SuccessFromFile;
		} else
			std::cerr << "Failed to load TLE data from backup file" << std::endl;
	} else
		std::cout << "Backup file does not exist: " << backupFilePath << std::endl;

	// Если существует база, используем ее
	if (getSatelliteCount() > 0) {
		std::cout << "Using existing database data despite update failure" << std::endl;
		return UpdateResult::UsingCachedData;
	}

	std::cerr << "Failed to update TLE data from any source" << std::endl;
	return UpdateResult::Failed;
}

bool DataManager::forceUpdateTleFromNetwork()
{
	std::cout << "Forcing TLE update from network..." << std::endl;
	auto data = downloadTleData();
	if (data.has_value() && !data->empty()) {
		saveTleDataToFile(data.value(), "celestrak_backup.tle");
		return updateTleFromString(data.value());
	}
	return false;
}

bool DataManager::forceUpdateTleFromFile(const std::string& filePath)
{
	std::cout << "Forcing TLE update from file: " << filePath << std::endl;
	return updateTleFromFile(filePath);
}

int DataManager::getSatelliteCount()
{
	return database->getSatelliteCount();
}

std::vector<SatelliteTle> DataManager::getAllSatellites()
{
	return database->getAllSatellites();
}

std::vector<SatelliteTle> DataManager::getSatellitesByGroup(const std::string& groupName)
{
	return groupManager->getSatellitesInGroup(groupName);
}

std::optional<SatelliteTle> DataManager::getSatelliteByNoradId(int noradId)
{
	return database->getSatelliteByNoradId(noradId);
}

std::optional<std::chrono::system_clock::time_point> DataManager::getLastTleUpdateTime()
{
	return database->getLastUpdateTime();
}

bool DataManager::addSatelliteToGroup(int noradId, const std::string& groupName)
{
	return database->addSatelliteToGroup(noradId, groupName);
}

bool DataManager::removeSatelliteFromGroup(int noradId, const std::string& groupName)
{
	return database->removeSatelliteFromGroup(noradId, groupName);
}

std::vector<std::string> DataManager::getSatelliteGroups(int noradId)
{
	return database->getSatelliteGroups(noradId);
}

std::vector<std::string> DataManager::getAllGroupNames()
{
	return groupManager->getGroupNames();
}

bool DataManager::sortSatellitesIntoGroups()
{
	if (!groupManager) {
		groupManager = std::make_unique<GroupManager>(database);
		if (!groupManager->initialize()) {
			std::cerr << "Failed to initialize GroupManager" << std::endl;
			return false;
		}
	}
	return groupManager->sortSatellitesIntoGroups();
}

std::optional<std::string> DataManager::downloadTleData()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	CURL* curl = curl_easy_init();
	if (!curl) {
		std::cerr << "Failed to initialize CURL" << std::endl;
		return std::nullopt;
	}

	std::string url = "https://celestrak.org/NORAD/elements/gp.php?GROUP=active&FORMAT=tle";
	std::string responseData;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "SatelliteTracker/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 секунд таймаут
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK) {
		std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
		return std::nullopt;
	}

	if (responseData.empty()) {
		std::cerr << "Empty response from Celestrak" << std::endl;
		return std::nullopt;
	}

	return responseData;
}

bool DataManager::updateTleFromString(const std::string& data)
{
	if (data.empty()) {
		std::cerr << "Empty TLE data string" << std::endl;
		return false;
	}

	auto satellites = parser.parseTleData(data);
	if (satellites.empty()) {
		std::cerr << "No satellites parsed from data string" << std::endl;
		return false;
	}

	std::cout << "Parsed " << satellites.size() << " satellites from data string" << std::endl;
	return updateDatabaseWithSatellites(satellites);
}

bool DataManager::updateTleFromNetwork()
{
	std::cout << "Downloading TLE data from network" << std::endl;
	auto data = downloadTleData();
	if (!data.has_value() || data->empty()) {
		std::cerr << "Network download failed or returned empty data" << std::endl;
		return false;
	}
	return updateTleFromString(data.value());
}

bool DataManager::updateTleFromFile(const std::string& filePath)
{
	std::cout << "Loading TLE data from file: " << filePath << std::endl;

	if (!std::filesystem::exists(filePath)) {
		std::cerr << "TLE file does not exist: " << filePath << std::endl;
        return false;
	}

	auto satellites = parser.parseTleFile(filePath);
	if (satellites.empty()) {
		std::cerr << "No satellites parsed from file: " << filePath << std::endl;
		return false;
	}

	std::cout << "Parsed " << satellites.size() << " satellites from file" << std::endl;
	return updateDatabaseWithSatellites(satellites);
}

bool DataManager::updateDatabaseWithSatellites(const std::vector<SatelliteTle>& satellites)
{
	if (satellites.empty()) {
		std::cerr << "No satellites to update" << std::endl;
		return false;
	}

	std::cout << "Updating database with " << satellites.size() << " satellites..." << std::endl;

	if (!database->beginTransaction()) {
		std::cerr << "Failed to begin database transaction" << std::endl;
		return false;
	}

	int insertedCount = 0;
	int updatedCount = 0;
	int failedCount = 0;

	for (const auto& satellite : satellites) {
		auto existingSatellite = getSatelliteByNoradId(satellite.noradId);

		if (existingSatellite.has_value()) {
			// Если существует - обновляем, иначе - вставляем новый
			if (database->updateSatellite(satellite))
				updatedCount++;
			else {
				std::cerr << "Failed to update satellite: " << satellite.name 
					<< " (NORAD: " << satellite.noradId << ")" << std::endl;
				failedCount++;
			}
		}
		else {
			if (database->insertSatellite(satellite))
				insertedCount++;
			else {
				std::cerr << "Failed to insert satellite: " << satellite.name
					<< " (NORAD: " << satellite.noradId << ")" << std::endl;
				failedCount++;
			}
		}
	}

	if (failedCount > 0) {
		std::cerr << failedCount << " satellite operations failed" << std::endl;
	}

	if (!database->commitTransaction()) {
		std::cerr << "Failed to commit transaction" << std::endl;
		return false;
	}

	std::cout << "Database update complete: " << insertedCount << " inserted, "
		<< updatedCount << " updated, "	<< failedCount << " failed" << std::endl;

	if (eventBus)
		eventBus->publish("TLE_DATA_UPDATED");

	return true;
}

bool DataManager::saveTleDataToFile(const std::string& data, const std::string& filePath)
{
	if (data.empty()) {
		std::cerr << "Cannot save empty data to file" << std::endl;
		return false;
	}

	auto parentPath = std::filesystem::path(filePath).parent_path();
	if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
		if (!std::filesystem::create_directories(parentPath)) {
			std::cerr << "Failed to create directory: " << parentPath << std::endl;
			return false;
		}
	}

	std::ofstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for writing: " << filePath << std::endl;
		return false;
	}

	file << data;
	file.close();

	if (file.fail()) {
		std::cerr << "Failed to write data to file: " << filePath << std::endl;
		return false;
	}

	return true;
}

size_t DataManager::writeCallback(void* contents, size_t size, size_t nmemb, std::string* data)
{
	size_t totalSize = size * nmemb;
	data->append((char*)contents, totalSize);
	return totalSize;
}
