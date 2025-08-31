#pragma once

#include <sqlite/sqlite3.h>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <sstream>
#include <ctime>

struct SatelliteTle {
	int id;
	std::string name;
	int noradId;
	std::string tleLine1;
	std::string tleLine2;
	std::string epoch;
};

struct SatelliteGroup {
	std::string name;
	std::vector<int> noradIds;
	bool isVisible = true;
};

class Database
{
public:
	Database(const std::string& path);
	~Database();

	bool isOpen() const;

	bool createTables();

	// Работа со спутниками
	bool insertSatellite(const SatelliteTle& satellite);
	bool updateSatellite(const SatelliteTle& satellite);
	bool deleteSatellite(int noradId);
	
	std::vector<SatelliteTle> getAllSatellites();
	
	std::optional<SatelliteTle> getSatelliteByNoradId(int noradId);
	std::vector<SatelliteTle> getSatellitesByGroup(const std::string& group);

	int getSatelliteCount();
	bool clearAllData();

	// Работа с группами
	//bool createGroup(const std::string& groupName);
	bool deleteGroup(const std::string& groupName);
	std::vector<std::string> getAllGroups();

	bool addSatelliteToGroup(int noradId, const std::string& group);
	bool removeSatelliteFromGroup(int noradId, const std::string& group);
	std::vector<std::string> getSatelliteGroups(int noradId);

	std::optional<std::chrono::system_clock::time_point> getLastUpdateTime();

	bool beginTransaction();
	bool commitTransaction();
	bool rollbackTransaction();

protected:
	bool open();
	void close();

private:
	bool executeSQL(const std::string& sql);

	std::string dbPath;
	sqlite3* db = nullptr;
	bool isTransactionActive = false;
};