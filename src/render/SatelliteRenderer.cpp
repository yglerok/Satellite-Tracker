#include "SatelliteRenderer.h"
#include "../satellitesManagement/Sgp4Model.h"

#include <iomanip>
#include <cmath>

SatelliteRenderer::SatelliteRenderer() : satelliteColor(1.0f, 0.0f, 0.0f),
	satelliteSize(10.0f), orbitColor(0.0f, 0.0f, 1.0f), orbitSegments(100),
	orbitDurationHours(2.0), showOrbits(true), showSatellites(true)
{
}

SatelliteRenderer::~SatelliteRenderer()
{
	glDeleteVertexArrays(1, &satelliteVAO);
	glDeleteBuffers(1, &satelliteVBO);
	glDeleteVertexArrays(1, &orbitVAO);
	glDeleteBuffers(1, &satelliteVBO);
	glDeleteProgram(satelliteShader);
	glDeleteProgram(orbitShader);
}

bool SatelliteRenderer::initialize()
{
	if (!compileShaders()) {
		std::cerr << "Failed to compile shaders for SatelliteRenderer" << std::endl;
		return false;
	}

	setupSatelliteBuffers();
	setupOrbitBuffers();

	return true;
}

void SatelliteRenderer::renderSatellites(const glm::mat4& view, const glm::mat4& projection, 
	const std::vector<SatelliteState>& satellites, bool onlyVisible)
{
	if (!showSatellites || satellites.empty()) {
		std::cout << "No satellites to render or rendering disabled" << std::endl;
		return;
	}

	glUseProgram(satelliteShader);

	Shader::setMat4(satelliteShader, "view", view);
	Shader::setMat4(satelliteShader, "projection", projection);
	Shader::setVec3(satelliteShader, "color", satelliteColor);
	Shader::setFloat(satelliteShader, "size", satelliteSize);

	glBindVertexArray(satelliteVAO);

	int renderCount = 0;

	for (const auto& satellite : satellites) {
		if (onlyVisible && !satellite.isVisible)
			continue;

		renderSatellite(satellite, view, projection);
		renderCount++;
	}

	glBindVertexArray(0);
}

void SatelliteRenderer::renderOrbits(const glm::mat4& view, const glm::mat4& projection, 
	const std::vector<SatelliteState>& satellites, 
	const std::map<int, std::shared_ptr<struct Sgp4Model>>& models, double currentJulianDate)
{
	if (!showOrbits || satellites.empty())
		return;

	updateOrbitCache(models, currentJulianDate);

	glUseProgram(orbitShader);

	Shader::setMat4(orbitShader, "view", view);
	Shader::setMat4(orbitShader, "projection", projection);
	Shader::setVec3(orbitShader, "color", orbitColor);

	glBindVertexArray(orbitVAO);

	for (const auto& satellite : satellites) {
		if (!satellite.isVisible)
			continue;

		auto orbitIt = orbitCache.find(satellite.noradId);
		if (orbitIt == orbitCache.end())
			continue;

		const auto& orbitPoints = orbitIt->second;
		if (orbitPoints.size() < 2)
			continue;

		// Обновляем буфер с точками орбиты
		glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
		glBufferData(GL_ARRAY_BUFFER, orbitPoints.size() * sizeof(glm::vec3), orbitPoints.data(), GL_STATIC_DRAW);
		
		glDrawArrays(GL_LINE_STRIP, 0, orbitPoints.size());
	}

	glBindVertexArray(0);
}

bool SatelliteRenderer::compileShaders()
{
	satelliteShader = Shader::create("res/shaders/satellite.vert", "res/shaders/satellite.frag");
	if (satelliteShader == 0)
		return false;

	orbitShader = Shader::create("res/shaders/orbit.vert", "res/shaders/orbit.frag");
	if (orbitShader == 0)
		return false;

	return true;
}

void SatelliteRenderer::setupSatelliteBuffers()
{
	// Создаем простой квадрат для рендера точек (будет увеличен в геометрическом шейдере)
	satellitesVertices = { glm::vec3(0.0f, 0.0f, 0.0f) };

	glGenVertexArrays(1, &satelliteVAO);
	glGenBuffers(1, &satelliteVBO);

	glBindVertexArray(satelliteVAO);
	glBindBuffer(GL_ARRAY_BUFFER, satelliteVBO);
	glBufferData(GL_ARRAY_BUFFER, satellitesVertices.size() * sizeof(glm::vec3),
		satellitesVertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);
}

void SatelliteRenderer::setupOrbitBuffers()
{
	glGenVertexArrays(1, &orbitVAO);
	glGenBuffers(1, &orbitVBO);

	glBindVertexArray(orbitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	glBindVertexArray(0);
}

void SatelliteRenderer::renderSatellite(const SatelliteState& satellite, const glm::mat4& view, const glm::mat4& projection)
{
	// Масштабируем для Earth radius = 1.0
	glm::vec3 scaledPosition = glm::vec3(satellite.positionEcef) / 6371.0f;
	/*std::cout << "  Scaled position: " << scaledPosition.x << ", "
		<< scaledPosition.y << ", " << scaledPosition.z << std::endl;*/

	scaledPosition = glm::vec3(scaledPosition.x, scaledPosition.z, -scaledPosition.y);

	// Создаем матрицу модели
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, scaledPosition);

	// Полное преобразование
	glm::mat4 mvp = projection * view * model;
	glm::vec4 clipPos = mvp * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// Деление на W для получения NDC координат
	glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;


	//std::cout << "NDC position: " << ndcPos.x << ", " << ndcPos.y << ", " << ndcPos.z << std::endl;

	// Если координаты в допустимом диапазоне, рисуем
	if (ndcPos.z > -1.0f && ndcPos.z < 1.0f) {
		Shader::setMat4(satelliteShader, "model", model);
		glDrawArrays(GL_POINTS, 0, 1);
	}
}

void SatelliteRenderer::calcOrbits(const std::shared_ptr<struct Sgp4Model>& model, double startTimeJd, 
	double durationHours, int segments, std::vector<glm::vec3>& outPoints)
{
	outPoints.clear();
	if (!model || !model->isValid())
		return;

	outPoints.reserve(segments + 1);

	double epochJd = model->getEpochJulianDate();
	double durationMinutes = durationHours * 60.0;
	double timeStep = durationMinutes / segments;

	double startMinutesFromEpoch = (startTimeJd - epochJd) * 24.0 * 60.0;

	for (int i = 0; i <= segments; ++i) {
		double minutesSinceEpoch = startMinutesFromEpoch + i * timeStep;
		if (std::abs(minutesSinceEpoch) > 7 * 24 * 60) { // 7 дней
			return;
		}

		glm::dvec3 positionTeme, velocityTeme;
		if (model->calcPosition(minutesSinceEpoch, positionTeme, velocityTeme)) {
			// Текущая юлианская дата для данной точки орбиты
			double pointTimeJd = epochJd + (minutesSinceEpoch / (24.0 * 60.0));

			// TEME -> ECEF
			glm::dvec3 positionEcef, velocityEcef;
			SatelliteManager::temeToEcef(pointTimeJd, positionTeme, velocityTeme, positionEcef, velocityEcef);

			// Масштабируем позицию
			glm::dvec3 scaledPosition = positionEcef / 6371.0;

			outPoints.push_back(glm::vec3(scaledPosition));
		}
	}
	// Если орбита получилась слишком короткой, добавляем точки
	if (outPoints.size() < 2) {
		std::cerr << "Warning: Orbit calculation failed for satellite" << std::endl;
		outPoints.clear();
	}

}

void SatelliteRenderer::updateOrbitCache(const std::map<int, std::shared_ptr<struct Sgp4Model>>& models,
	double currentJulianDate)
{
	// Очищаем кэш для неактивных спутников
	std::vector<int> toRemove;

	for (const auto& [noradId, _] : orbitCache) {
		if (models.find(noradId) == models.end())
			toRemove.push_back(noradId);
	}

	for (int id : toRemove)
		orbitCache.erase(id);

	// Обновляем орбиты для всех моделей
	for (const auto& [noradId, model] : models) {
		// Пересчитываем орбиту, если ее нет в кэше или прошло много времени
		if (orbitCache.find(noradId) == orbitCache.end()) {
			std::vector<glm::vec3> orbitPoints;
			calcOrbits(model, currentJulianDate, orbitDurationHours, orbitSegments, orbitPoints);
			orbitCache[noradId] = orbitPoints;
		}
	}
}
