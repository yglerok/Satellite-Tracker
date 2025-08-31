#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Database.h"

class GroupManager
{
public:
	GroupManager(std::shared_ptr<Database> database);

	bool initialize();

	bool createGroup(const std::string& groupName);
	bool deleteGroup(const std::string& groupName);
	bool addSatelliteToGroup(int noradId, const std::string& groupName);
	//bool removeSatelliteFromGroup(int noradId, const std::string& groupName);

	// Автоматическая сортировка по группам
	bool sortSatellitesIntoGroups();

	// Получение данных
	std::vector<std::string> getGroupNames() const;
	std::vector<SatelliteTle> getSatellitesInGroup(const std::string& groupName) const;
	//std::vector<int> getNoradIdsInGroup(const std::string& groupName) const;

	// Управление видимостью
	void setGroupVisibility(const std::string& groupName, bool visible);
	bool isGroupVisible(const std::string& groupName) const;

private:
	// Правила для автоматической сортировки
	bool isStarlinkSatellite(const SatelliteTle& satellite) const;
	bool isWeatherSatellite(const SatelliteTle& satellite) const;
	bool isGNSSSatellite(const SatelliteTle& satellite) const;
	bool isSpaceStation(const SatelliteTle& satellite) const;
	bool isScientificSatellite(const SatelliteTle& satellite) const;
	void categorizeByOrbit(const SatelliteTle& satellite);

	std::shared_ptr<Database> database;
	std::unordered_map<std::string, SatelliteGroup> groups;
};