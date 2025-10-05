#pragma once

#include <glad/glad.h>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <unordered_map>

#include "render/Earth.h"
#include "render/Shaders.h"
#include "render/Sun.h"
#include "render/SatelliteRenderer.h"

#include "Camera.h"
#include "TimeManager.h"

struct InputParameters {
	float ambientStrength = 0.05f, specularStrength = 0.1f;
	float //lightPos[3] = { 2.0f, 3.0f, 3.0f },
		lightColor[3] = { 1.0f, 1.0f, 1.0f };
	float nightTextureIntensity = 0.4f;
};

class Application
{
public:
	Application() = delete;
	Application(const char* appTitle, int appWidth, int appHeight);
	~Application() = default;
	
	bool init();
	void start();
	void shutdown();

private:
	std::string title;
	int width, height;
	bool isRunning = true;
	const int FPS = 60;
	const double satelliteUpdateInterval = 0.5;
	std::chrono::steady_clock::time_point lastUpdateTime;

	bool isFullscreen = false;
	int displayWidth = 1920;   // FHD по умолчанию
	int displayHeight = 1080;

	void toggleFullscreen();
	void updateWindowSize();
		
	void processInput();
	void update(double dt);
	void render();

	SDL_Window* window = nullptr;
	SDL_GLContext context;
	GLuint shaderProgram = 0;
	std::unique_ptr<UI> ui;
	Camera* camera;
	Earth* earth = nullptr;
	std::unique_ptr<Sun> sun;
	glm::vec3 sunDir;

	const float mouseSensitivity = 0.01f;

	MouseState mouseState;
	InputParameters inputParams;
	RenderOptions renderOptions;
	std::unordered_map<std::string, int> groupSize; // Save groups size here for GUI filters

	std::atomic<bool> updateInProgress = false;
	std::future<void> updateFuture;
	std::mutex dataMutex;
	std::vector<SatelliteState> cachedSatelliteStates;
	std::map<int, std::vector<glm::vec3>> orbitCache;

	std::shared_ptr<DataManager> dataManager;
	void loadDataFromDatabase();
	const std::string dbPath = "data/satellites.db";
	const std::string backupPath = "data/backups/celestrak_backup.tle";

	std::shared_ptr<SatelliteManager> satelliteManager;
	std::unique_ptr<SatelliteRenderer> satelliteRenderer;

	std::shared_ptr<TimeManager> timeManager;
	std::unique_ptr<EventBus> eventBus;

	float calculateSatelliteSize();

	bool initializeManagers();
};