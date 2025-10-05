#pragma once

#include "Sgp4ModelFactory.h"
#include "../data/DataManager.h"

#include <memory>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <unordered_map>

class EventBus; // Класс для связи dataManager с SatelliteManager

// Структура для передачи данных в рендер
struct SatelliteState {
	glm::dvec3 positionEcef; // Позиция в системе ECEF (Earth-Centered, Earth-Fixed)
    glm::dvec3 velocityEcef; // Скорость в ECEF
    int noradId;
    std::string name;
    bool isVisible; // Учитывает фильтры (группы и т.д.)
};

class SatelliteManager
{
public:
    SatelliteManager(std::shared_ptr<DataManager> manager) : dataManager(manager) {}
    ~SatelliteManager() = default;

    bool initialize();
    void update(double utcJd); // Главный метод обновления на заданное Юлианское время (UTC)

    // Подписка на события обновления данных
    void setEventBus(EventBus* bus);

    const std::vector<SatelliteState> getSatelliteStates() const { return satelliteStates; }
    const SatelliteState* getSatelliteState(int noradId) const;

    const std::map<int, std::vector<glm::vec3>> getOrbitCache() const { return orbitCache; }

    // Преобразование координат из TEME в ECEF
    // TEME - система координат, зафиксированная относительно звезд. Ее оси не вращаются с Землей
    // ECEF - система координат, которая вращается с Землей. Ось Z направлена на северный полюс, X - на гринвичский меридиан
    // Преобразование заключается в повороте вокруг оси Z на угол, равный гринвичкому звездному времени
    static void temeToEcef(double utcJd, const glm::dvec3& posTeme, const glm::dvec3& velTeme,
        glm::dvec3& posEcef, glm::dvec3& velEcef);

    // Простейшая фильтрация
    void setGroupFilters(const std::unordered_map<std::string, bool>& groupNames);
    void clearFilter();

    const std::map<int, std::shared_ptr<Sgp4Model>>& getModels() const { return models; }

private:
    struct SatelliteData {
        std::shared_ptr<const Sgp4Model> model; // Неизменяемая модель
        SatelliteState currentState;
        bool isActive = true;
    };

    std::map<int, SatelliteData> satelliteData;
    std::map<int, std::shared_ptr<Sgp4Model>> models;

    // Callback для получения уведомлений от DataManager
    void onTleDataUpdated();
    // Загрузка спутников из БД и создание моделей
    void loadModelsFromDatabase();

    bool isSatelliteVisible(int noradId) const;

    std::map<int, std::vector<glm::vec3>> orbitCache; // Кэш рассчитанных орбит по NORAD ID
    
    std::shared_ptr<DataManager> dataManager;
    EventBus* eventBus = nullptr;

    std::vector<SatelliteState> satelliteStates; // Актуальный список для рендерера

    std::unordered_map<std::string, bool> currentFilterGroups;
};