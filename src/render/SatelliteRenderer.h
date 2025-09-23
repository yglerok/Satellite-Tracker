#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <map>

#include "Shaders.h"
#include "../satellitesManagement/SatelliteManager.h"

class SatelliteRenderer
{
public:
	SatelliteRenderer();
	~SatelliteRenderer();

	bool initialize();
	void renderSatellites(const glm::mat4& view, const glm::mat4& projection,
		const std::vector<SatelliteState>& satellites, bool onlyVisible);
	void renderOrbits(const glm::mat4& view, const glm::mat4& projection,
		const std::vector<SatelliteState>& satellites,
		const std::map<int, std::shared_ptr<struct Sgp4Model>>& models,
		double currentJulianDate);

	// Настройка внешнего вида
	void setSatelliteColor(const glm::vec3& color) { satelliteColor = color; };
	void setSatelliteSize(float size) { satelliteSize = size; }
	void setOrbitColor(const glm::vec3& color) { orbitColor = color; }
	void setOrbitSegmentCount(int count) { orbitSegments = count; }
	void setOrbitDurationHours(double hours) { orbitDurationHours = hours; }

	void toggleOrbits(bool enabled) { showOrbits = enabled; }
	void toggleSatellites(bool enabled) { showSatellites = enabled; }

private:
	// Для рендера спутников
	GLuint satelliteShader;
	GLuint satelliteVAO, satelliteVBO;
	std::vector<glm::vec3> satellitesVertices;

	// Для рендера орбит
	GLuint orbitShader;
	GLuint orbitVAO, orbitVBO;
	std::map<int, std::vector<glm::vec3>> orbitCache; // Кэш рассчитанных орбит по NORAD ID

	// Настройка рендера
	glm::vec3 satelliteColor;
	float satelliteSize;
	glm::vec3 orbitColor;
	int orbitSegments;
	double orbitDurationHours;
	bool showOrbits;
	bool showSatellites;

	bool compileShaders();
	void setupSatelliteBuffers();
	void setupOrbitBuffers();

	void renderSatellite(const SatelliteState& satellite, const glm::mat4& view,
		const glm::mat4& projection);

	void calcOrbits(const std::shared_ptr<struct Sgp4Model>& model,
		double startTimeJd, double durationHours, int segments, std::vector<glm::vec3>& outPoints);
	void updateOrbitCache(const std::map<int, std::shared_ptr<struct Sgp4Model>>& models,
		double currentJulianDate);
};