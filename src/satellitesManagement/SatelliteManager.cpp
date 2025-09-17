#include "SatelliteManager.h"

#include <algorithm>

bool SatelliteManager::initialize()
{
	if (!dataManager) {
		std::cerr << "Cannot initialize SatelliteManager: No DataManager provided!" << std::endl;
		return false;
	}
	loadModelsFromDatabase();
	return true;
}

void SatelliteManager::update(double utcJd)
{
	std::unique_lock<std::mutex> ul(mtx, std::defer_lock);

	visibleSatelliteStates.clear();
	visibleSatelliteStates.reserve(satelliteData.size());

	//std::cout << "Current UTC JD: " << utcJd << std::endl;

	for (auto& [noradId, satData] : satelliteData) {
		if (!satData.isActive)
			continue;

		double epochJD = satData.model->getEpochJulianDate();
		//std::cout << "Current epochJD: " << epochJD << std::endl;
		// 1. Рассчитываем время, прошедшее с эпохи TLE (в минутах)
		double minutesSinceEpoch = (utcJd - epochJD) * 24.0 * 60.0;

		if (std::abs(minutesSinceEpoch) > 7 * 24 * 60) { // 7 дней
			continue;
		}

		// 2. Вычисляем положение в TEME
		glm::dvec3 positionTeme, velocityTeme;
		if (satData.model->calcPosition(minutesSinceEpoch, positionTeme, velocityTeme)) {
			// 3. Преобразуем TEME -> ECEF
			glm::dvec3 positionEcef, velocityEcef;
			temeToEcef(utcJd, positionTeme, velocityTeme, positionEcef, velocityEcef);

			// 4. Обновляем состояние
			satData.currentState.positionEcef = positionEcef;
			satData.currentState.velocityEcef = velocityEcef;

			// 5. Применяем фильтр
			satData.currentState.isVisible = isSatelliteVisible(noradId);

			// 6. Добавляем в список видимых
			if (satData.currentState.isVisible) {
				ul.lock();
				visibleSatelliteStates.push_back(satData.currentState);
				ul.unlock();
			}
				
			

		}
		else {
			std::cerr << "Failed to calculate position for satellite: " << satData.currentState.name << std::endl;
			satData.isActive = false;
		}
				
	}
}

void SatelliteManager::setEventBus(EventBus* bus)
{
	eventBus = bus;
	if (eventBus)
		eventBus->subscribe("TLE_DATA_UPDATED", [this]() {
		this->onTleDataUpdated();
			});
}

const SatelliteState* SatelliteManager::getSatelliteState(int noradId) const
{
	auto it = satelliteData.find(noradId);
	if (it != satelliteData.end())
		return &it->second.currentState;
	else
		return nullptr;
}

void SatelliteManager::setGroupFilter(const std::string& groupName)
{
	currentFilterGroup = groupName;
	std::cout << "Filter set to group: " << groupName << std::endl;
}

void SatelliteManager::clearFilter()
{
	currentFilterGroup.clear();
	std::cout << "Filter cleared" << std::endl;
}

void SatelliteManager::onTleDataUpdated()
{
	std::cout << "SatelliteManager: TLE data updated, reloading models..." << std::endl;
	loadModelsFromDatabase();
}

void SatelliteManager::loadModelsFromDatabase()
{
	satelliteData.clear();
	models.clear();
	visibleSatelliteStates.clear();

	auto satellites = dataManager->getAllSatellites();
	std::cout << "Loading " << satellites.size() << " satellite models..." << std::endl;

	for (const auto& satellite : satellites) {
		auto model = Sgp4ModelFactory::create(satellite.tleLine1, satellite.tleLine2);
		if (model && model->isValid()) {
			SatelliteData data;
			data.model = model;
			data.currentState.noradId = satellite.noradId;
			data.currentState.name = satellite.name;
			data.isActive = true;

			satelliteData[satellite.noradId] = std::move(data);
			models[satellite.noradId] = model;
		}
		else 
			std::cerr << "Failed to create model for: " << satellite.name << std::endl;
	}
	std::cout << "Successfully loaded " << satelliteData.size() << " models." << std::endl;
}

void SatelliteManager::temeToEcef(double utcJd, const glm::dvec3& posTeme, const glm::dvec3& velTeme, 
	glm::dvec3& posEcef, glm::dvec3& velEcef)
{
	double gst = gstime(utcJd);

	double cosg = cos(gst);
	double sing = sin(gst);

	// Поворачиваем позицию
	posEcef.x = posTeme.x * cosg + posTeme.y * sing;
	posEcef.y = -posTeme.x * sing + posTeme.y * cosg;
	posEcef.z = posTeme.z;

	// Поворачиваем скорость
	velEcef.x = velTeme.x * cosg + velTeme.y * sing;
	velEcef.y = -velTeme.x * sing + velTeme.y * cosg;
	velEcef.z = velTeme.z;
}

bool SatelliteManager::isSatelliteVisible(int noradId) const
{
	if (currentFilterGroup.empty())
		return true;

	// Проверяем, принадлежит ли спутник к выбранной группе. Делаем запрос к DataManager
	auto groups = dataManager->getSatelliteGroups(noradId);

	return std::find(groups.begin(), groups.end(), currentFilterGroup) != groups.end();
}
