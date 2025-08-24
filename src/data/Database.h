#pragma once

#include <sqlite/sqlite3.h>
#include <string>
#include <vector>
#include <optional>

struct SatelliteTle {
	int id;
	std::string name;
	int noradId;
	std::string tleLine1;
	std::string tleLine2;
	std::string epoch;
};

class Database
{
public:
	Database(const std::string& path);
	~Database();

	bool isOpen() const;

	bool createTables();
	bool insertSatellite(const SatelliteTle& satellite);
	bool updateSatellite(const SatelliteTle& satellite);
	bool deleteSatellite(int noradId);

	std::optional<SatelliteTle> getSatelliteByNoradId(int noradId);
	std::vector<SatelliteTle> getAllSatellites();
	std::vector<SatelliteTle> getSatellitesByGroups(const std::string& group);

	int getSatelliteCount();
	bool clearAllData();

	bool addSatelliteToGroup(int noradId, const std::string& group);
	bool removeSatelliteFromGroup(int noradId, const std::string& group);
	std::vector<std::string> getSatelliteGroups(int noradId);

protected:
	bool open();
	void close();

private:
	bool executeSQL(const std::string& sql);
	bool beginTransaction();
	bool commitTransaction();
	bool rollbackTransaction();

	std::string dbPath;
	sqlite3* db = nullptr;
	bool isTransactionActive = false;
};