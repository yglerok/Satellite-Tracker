#include "SatelliteRenderer.h"
#include "../satellitesManagement/Sgp4Model.h"

#include <iomanip>
#include <cmath>

SatelliteRenderer::SatelliteRenderer() : satelliteColor(1.0f, 0.0f, 0.0f),
	satelliteSize(2.0f), orbitColor(1.0f, 1.0f, 1.0f), orbitSegments(100),
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

	glEnable(GL_PROGRAM_POINT_SIZE);

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
	const std::vector<SatelliteState>& satellites, std::map<int, std::vector<glm::vec3>> orbitCache,
	bool onlyVisible)
{
	if (!showOrbits || satellites.empty())
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(orbitShader);

	Shader::setMat4(orbitShader, "view", view);
	Shader::setMat4(orbitShader, "projection", projection);
	Shader::setVec3(orbitShader, "color", orbitColor);

	glBindVertexArray(orbitVAO);

	for (const auto& satellite : satellites) {
		if (onlyVisible && !satellite.isVisible)
			continue;

		auto orbitIt = orbitCache.find(satellite.noradId);
		if (orbitIt == orbitCache.end())
			continue;

		const auto& orbitPoints = orbitIt->second;

		// Обновляем буфер с точками орбиты
		glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
		glBufferData(GL_ARRAY_BUFFER, orbitPoints.size() * sizeof(glm::vec3), orbitPoints.data(), GL_STATIC_DRAW);
		
		glDrawArrays(GL_LINE_STRIP, 0, orbitPoints.size());
	}

	glBindVertexArray(0);
	glDisable(GL_BLEND);
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

