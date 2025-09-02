#pragma once

#include <string>
#include <chrono>
#include <memory>

#include <curl/curl.h>

#include "Database.h"
#include "TleParser.h"
#include "GroupManager.h"
#include "../EventBus.h"

class DataManager
{
public:
	DataManager(const std::string& dbPath);
	~DataManager();

	bool initialize();

	enum class UpdateResult {
		SuccessFromNetwork,
		SuccessFromFile,
		UsingCachedData,
		Failed
	};

	UpdateResult updateTleData(const std::string& backupFilePath,
						const std::chrono::hours& updateInterval = std::chrono::hours(24));
	bool forceUpdateTleFromNetwork();
	bool forceUpdateTleFromFile(const std::string& filePath);

	int getSatelliteCount();
	std::vector<SatelliteTle> getAllSatellites();
	std::vector<SatelliteTle> getSatellitesByGroup(const std::string& groupName);
	std::optional<SatelliteTle> getSatelliteByNoradId(int noradId);
	std::optional<std::chrono::system_clock::time_point> getLastTleUpdateTime();

	bool addSatelliteToGroup(int noradId, const std::string& groupName);
	bool removeSatelliteFromGroup(int noradId, const std::string& groupName);
	std::vector<std::string> getSatelliteGroups(int noradId);
	std::vector<std::string> getAllGroupNames();
	bool sortSatellitesIntoGroups();

	void setEventBus(EventBus* bus) {
		eventBus = bus;
	}

private:
	std::optional<std::string> downloadTleData();

	bool updateTleFromString(const std::string& data);
	bool updateTleFromNetwork();
	bool updateTleFromFile(const std::string& filePath);
	bool updateDatabaseWithSatellites(const std::vector<SatelliteTle>& satellites);
	bool saveTleDataToFile(const std::string& data, const std::string& filePath);

	std::shared_ptr<Database> database;
	TleParser parser;
	std::unique_ptr<GroupManager> groupManager;

	static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* data);

	EventBus* eventBus;
};