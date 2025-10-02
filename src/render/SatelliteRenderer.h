#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <map>

#include "Shaders.h"
#include "../satellitesManagement/SatelliteManager.h"

#include "../UI.h"

class SatelliteRenderer
{
public:
	SatelliteRenderer(const RenderOptions& options);
	~SatelliteRenderer();

	bool initialize();
	void renderSatellites(const glm::mat4& view, const glm::mat4& projection,
		const std::vector<SatelliteState>& satellites, bool onlyVisible);
	void renderOrbits(const glm::mat4& view, const glm::mat4& projection,
		const std::vector<SatelliteState>& satellites, std::map<int, std::vector<glm::vec3>> orbitCache,
		bool onlyVisible);

	// Настройка внешнего вида
	void setSatelliteColor(const glm::vec4& color) { satelliteColor = color; };
	void setSatelliteSize(float size) { satelliteSize = size; }
	void setOrbitColor(const glm::vec4& color) { orbitColor = color; }

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

	// Настройка рендера
	glm::vec4 satelliteColor;
	float satelliteSize;
	glm::vec4 orbitColor;
	bool showOrbits;
	bool showSatellites;

	bool compileShaders();
	void setupSatelliteBuffers();
	void setupOrbitBuffers();

	void renderSatellite(const SatelliteState& satellite, const glm::mat4& view,
		const glm::mat4& projection);
};