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
	bool isGlobalstarSatellite(const SatelliteTle& satellite) const;
	bool isOrbcommSatellite(const SatelliteTle& satellite) const;
	bool isPlanetLabsSatellite(const SatelliteTle& satellite) const;
	bool isSpireSatellite(const SatelliteTle& satellite) const;
	bool isAmazonProjectKuiper(const SatelliteTle& satellite) const;
	bool isSwarmSatellite(const SatelliteTle& satellite) const;
	bool isPlanetSkySat(const SatelliteTle& satellite) const;
	bool isGPSSatellite(const SatelliteTle& satellite) const;
	bool isGLONASSSatellite(const SatelliteTle& satellite) const;
	bool isGalileoSatellite(const SatelliteTle& satellite) const;
	bool isBeiDouSatellite(const SatelliteTle& satellite) const;
	bool isIRNSSSatellite(const SatelliteTle& satellite) const;
	bool isQZSSSatellite(const SatelliteTle& satellite) const;
	bool isSBASSatellite(const SatelliteTle& satellite) const;
	bool isTelesatSatellite(const SatelliteTle& satellite) const;
	bool isInmarsatSatellite(const SatelliteTle& satellite) const;
	bool isThurayaSatellite(const SatelliteTle& satellite) const;
	bool isViasatSatellite(const SatelliteTle& satellite) const;
	bool isEchoStarSatellite(const SatelliteTle& satellite) const;
	bool isSiriusXMSatellite(const SatelliteTle& satellite) const;
	bool isABSsatellite(const SatelliteTle& satellite) const;
	bool isArabsatSatellite(const SatelliteTle& satellite) const;
	bool isAsiaSatSatellite(const SatelliteTle& satellite) const;
	bool isGazpromSatellite(const SatelliteTle& satellite) const;
	bool isNASASatellite(const SatelliteTle& satellite) const;
	bool isESASatellite(const SatelliteTle& satellite) const;
	bool isHubbleTelescope(const SatelliteTle& satellite) const;
	bool isJamesWebbTelescope(const SatelliteTle& satellite) const;
	bool isPlanetaryScienceSatellite(const SatelliteTle& satellite) const;
	bool isAstronomySatellite(const SatelliteTle& satellite) const;
	bool isJAXASatellite(const SatelliteTle& satellite) const;
	bool isISROSSatellite(const SatelliteTle& satellite) const;
	bool isROSCOSMOSSatellite(const SatelliteTle& satellite) const;
	bool isChineseAcademySciences(const SatelliteTle& satellite) const;
	bool isSpaceTelescope(const SatelliteTle& satellite) const;
	bool isSolarObservation(const SatelliteTle& satellite) const;
	bool isMagnetosphericResearch(const SatelliteTle& satellite) const;
	bool isNOAASatellite(const SatelliteTle& satellite) const;
	bool isGOESSatellite(const SatelliteTle& satellite) const;
	bool isMeteosatSatellite(const SatelliteTle& satellite) const;
	bool isFengyunSatellite(const SatelliteTle& satellite) const;
	bool isElectroLSatellite(const SatelliteTle& satellite) const;
	bool isMeteorMSatellite(const SatelliteTle& satellite) const;
	bool isJPSSSatellite(const SatelliteTle& satellite) const;
	bool isCOSMICsatellite(const SatelliteTle& satellite) const;
	bool isGCOMsatellite(const SatelliteTle& satellite) const;
	bool isUSMilitarySatellite(const SatelliteTle& satellite) const;
	bool isRussianMilitarySatellite(const SatelliteTle& satellite) const;
	bool isReconnaissanceSatellite(const SatelliteTle& satellite) const;
	bool isEarlyWarningSatellite(const SatelliteTle& satellite) const;
	bool isNROLsatellite(const SatelliteTle& satellite) const;
	bool isTopazSatellite(const SatelliteTle& satellite) const;
	bool isSARsatellite(const SatelliteTle& satellite) const;
	bool isSignalsIntelligence(const SatelliteTle& satellite) const;
	bool isLandsatSatellite(const SatelliteTle& satellite) const;
	bool isSentinelSatellite(const SatelliteTle& satellite) const;
	bool isSpotSatellite(const SatelliteTle& satellite) const;
	bool isHighResolutionImagingSatellite(const SatelliteTle& satellite) const;
	bool isCommercialImaging(const SatelliteTle& satellite) const;
	bool isHyperspectralImaging(const SatelliteTle& satellite) const;
	bool isAgriculturalMonitoring(const SatelliteTle& satellite) const;
	bool isSpaceStationRelated(const SatelliteTle& satellite) const;
	bool isDebrisOrRocketBody(const SatelliteTle& satellite) const;
	bool isOneWebSatellite(const SatelliteTle& satellite) const;
	bool isIridiumSatellite(const SatelliteTle& satellite) const;
	bool isIntelsatSatellite(const SatelliteTle& satellite) const;
	bool isSESsatellite(const SatelliteTle& satellite) const;
	bool isEutelsatSatellite(const SatelliteTle& satellite) const;
	bool isAmateurRadioSatellite(const SatelliteTle& satellite) const;
	bool isCubeSat(const SatelliteTle& satellite) const;
	bool isTechnologyDemoSatellite(const SatelliteTle& satellite) const;
	bool isNanosatellite(const SatelliteTle& satellite) const;
	void categorizeByOrbit(const SatelliteTle& satellite);

	std::shared_ptr<Database> database;
	std::unordered_map<std::string, SatelliteGroup> groups;
};