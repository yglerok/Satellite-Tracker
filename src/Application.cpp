#include "Application.h"

#include <iostream>
#include <cmath>
#include <filesystem>

Application::Application(const char* appTitle, int appWidth, int appHeight) :
	title(appTitle), width(appWidth), height(appHeight)
{
	camera = nullptr;
}

bool Application::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "ERROR [SDL] Can't initialize SDL!" << std::endl;
		return false;
	}

	// Получаем информацию о дисплее
	SDL_DisplayID display = SDL_GetPrimaryDisplay();
	const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
	if (!mode) {
		std::cerr << "Failed to get current display mode: " << SDL_GetError() << std::endl;
		return false;
	}
	displayWidth = mode->w;
	displayHeight = mode->h;

	std::cout << "Display resolution: " << displayWidth << "x" << displayHeight << std::endl;

	// Если дисплей меньше FHD, используем его разрешение
	if (displayWidth < 1920 || displayHeight < 1080) {
		std::cout << "Using display resolution: " << displayWidth << "x" << displayHeight << std::endl;
	}
	else {
		// Используем FHD
		displayWidth = 1920;
		displayHeight = 1080;
	}
	
	width = displayWidth;
	height = displayHeight;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL);

	// Настройка контекста openGL
	context = SDL_GL_CreateContext(window);

	if (!gladLoadGL()) {
		std::cerr << "ERROR [glad] Can't load GL!" << std::endl;
		return false;
	}

	//toggleFullscreen();

	if (!initializeManagers()) {
		std::cerr << "Failed to init App!" << std::endl;
		return false;
	}

	ui = std::make_unique<UI>(width, height, dataManager, satelliteManager, renderOptions, mouseState, groupSize);
	ui->initialize(window, &context);

	return true;
}

void Application::start()
{
	// Настройка OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Загрузка шейдера
	shaderProgram = Shader::create("res/shaders/earth.vert", "res/shaders/earth.frag");

	// Создание модели Земли
	earth = new Earth();

	// Создание источника света (Солнца)
	// Если в последствии буду передавать время в разные компоненты,
	// сделать здесь динамическое создание
	sun = std::make_unique<Sun>(timeManager);
	sun->setLightning(shaderProgram);
	sunDir = sun->getDirection();

	// Инициализация таймеров
	auto previousTime = std::chrono::steady_clock::now();
	//auto currentTime = previousTime;
	//double lag = 0.0;
	//constexpr double fixed_dt = 1.0 / 60.0;
	//constexpr double max_lag = 0.1; // Максимальный лаг 100ms

	SDL_GL_SetSwapInterval(1); // Включаем vsync

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//ui->drawLoadingWindow();
	//SDL_GL_SwapWindow(window);
	//SDL_Delay(50);
	//
	//// обрабатываем события SDL чтобы окно обновилось
	//SDL_PumpEvents();
	
	// Загрузка данных
	std::atomic<bool> isDataLoaded = false;
	std::thread dataLoadingThread([this, &isDataLoaded]() {
		loadDataFromDatabase();
		isDataLoaded = true;
	});
	
	while (!isDataLoaded.load()) {
		processInput();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ui->drawLoadingWindow();
		SDL_GL_SwapWindow(window);

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
	

	dataLoadingThread.join();

	while (isRunning) {
		auto currentTime = std::chrono::steady_clock::now();
		double realElapsedTime = std::chrono::duration<double>(currentTime - previousTime).count();
		previousTime = currentTime;

		processInput();
		
		update(realElapsedTime);

		render();

		SDL_GL_SwapWindow(window);
	}
}

void Application::shutdown()
{
	if (updateFuture.valid()) {
		try {
			updateFuture.wait();
			updateFuture.get();
		}
		catch (const std::exception& e) {
			std::cerr << "ERROR during shutdown wait " << e.what() << std::endl;
		}
	}

	delete camera;
	delete earth;
	glDeleteProgram(shaderProgram);
	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Application::toggleFullscreen()
{
	isFullscreen = !isFullscreen;

	if (isFullscreen) {
		// Полноэкранный оконный режим
		SDL_SetWindowFullscreen(window, true);
		std::cout << "Switched to fullscreen: " << width << "x" << height << std::endl;
	}
	else {
		// Оконный режим
		SDL_SetWindowFullscreen(window, false);
		// Возвращаем к FHD размеру
		SDL_SetWindowSize(window, 1920, 1080);
		//SDL_CenterWindowInDisplay(window, SDL_GetPrimaryDisplay());
		std::cout << "Switched to windowed mode" << std::endl;
	}

	// Обновляем камеру с новыми размерами
	if (camera) {
		// Нужно будет обновить камеру или пересоздать
		camera->updateProjection(width, height);
	}
}

void Application::updateWindowSize()
{
	// Получаем актуальный размер окна
	SDL_GetWindowSize(window, &width, &height);
	std::cout << "Window resized to: " << width << "x" << height << std::endl;

	// Обновляем viewport OpenGL
	glViewport(0, 0, width, height);

	// Обновляем проекцию камеры
	if (camera) {
		camera->updateProjection(width, height);
	}

	// Обновляем UI
	if (ui) {
		// Нужно будет добавить метод обновления размеров в UI
		ui->updateWindowSize(width, height);
	}
}

void Application::processInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		switch (event.type)
		{
		case SDL_EVENT_QUIT:
			isRunning = false;
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			updateWindowSize();
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			if (mouseState.isOnMenuScrollArea)
				break;

			camera->increaseRadius(event.wheel.y);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			if (event.button.button == SDL_BUTTON_RIGHT) {
				mouseState.isPressed = true;
				mouseState.prevX = event.motion.x;
				mouseState.prevY = event.motion.y;
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			if (event.button.button == SDL_BUTTON_RIGHT)
				mouseState.isPressed = false;
			break;
		case SDL_EVENT_MOUSE_MOTION: {
			if (!mouseState.isPressed)
				break;

			float dx = event.motion.x - mouseState.prevX;
			float dy = event.motion.y - mouseState.prevY;

			camera->increasePhi(dx * mouseSensitivity);
			camera->increaseTheta(dy * mouseSensitivity);

			mouseState.prevX = event.motion.x;
			mouseState.prevY = event.motion.y;
			break;
		}
		case SDL_EVENT_KEY_DOWN:
			if (event.key.scancode == SDL_SCANCODE_F11) {
				toggleFullscreen();
			}
			if (event.key.scancode == SDL_SCANCODE_R) {
				camera->reset();
			}
			break;
		default:
			break;
		}

	}
}

void Application::update(double dt)
{
	static double lastSatelliteCalculation = satelliteUpdateInterval;

	timeManager->update(dt);
	camera->update(dt);	

	glUseProgram(shaderProgram);
	
	// Настройка освещения
	Shader::setFloat(shaderProgram, "ambientStrength", inputParams.ambientStrength);
	Shader::setFloat(shaderProgram, "specularStrength", inputParams.specularStrength);
	Shader::setVec3(shaderProgram, "objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
	Shader::setVec3(shaderProgram, "lightColor", glm::vec3(inputParams.lightColor[0], inputParams.lightColor[1], inputParams.lightColor[2]));

	Shader::setFloat(shaderProgram, "nightIntensity", inputParams.nightTextureIntensity);
	
	lastSatelliteCalculation += dt;

	if (lastSatelliteCalculation >= satelliteUpdateInterval && !updateInProgress.load() &&
		(!updateFuture.valid() || updateFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
		updateInProgress = true;
		lastSatelliteCalculation = 0;

		updateFuture = std::async(std::launch::async, [this]() {
			try {
				satelliteManager->update(timeManager->getCurrentJulianDate());

				std::lock_guard<std::mutex> lock(dataMutex);
				cachedSatelliteStates = satelliteManager->getSatelliteStates();
				orbitCache = satelliteManager->getOrbitCache();
			}
			catch (const std::exception& e) {
				std::cerr << "ERROR in satellite update thread: " << e.what() << std::endl;
			}
			updateInProgress = false;
			});
	}


	if (updateFuture.valid() && updateFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
		try {
			updateFuture.get();
		}
		catch (const std::exception& e) {
			std::cerr << "ERROR getting satellite update result: " << e.what() << std::endl;
		}
	}

	
}

void Application::render()
{
	// Проверка ДО очистки
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL error before clear: " << err << std::endl;
	}

	glViewport(0, 0, width, height);

	// Очистка буферов
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->render();

	//earth->renderSunDirection(camera->getView(), camera->getProjection(), sunDir);

	// Отрисовка Земли
	earth->render(camera->getView(), camera->getProjection(), shaderProgram);

	// Вычисление размера спутников в зависимости от отдаления камеры (используем линейную интерполяцию)
	float satelliteSize = calculateSatelliteSize();
	satelliteRenderer->setSatelliteSize(satelliteSize);
	satelliteRenderer->setSatelliteColor(glm::vec4(renderOptions.satellitesColor[0], renderOptions.satellitesColor[1], 
		renderOptions.satellitesColor[2], renderOptions.satellitesColor[3]));
	satelliteRenderer->setOrbitColor(glm::vec4(renderOptions.orbitsColor[0], renderOptions.orbitsColor[1],
		renderOptions.orbitsColor[2], renderOptions.orbitsColor[3]));

	{
		std::lock_guard<std::mutex> lock(dataMutex);
		if (renderOptions.areSatellitesVisible)
			satelliteRenderer->renderSatellites(camera->getView(), camera->getProjection(), cachedSatelliteStates, true);
		if (renderOptions.areOrbitsVisible)
			satelliteRenderer->renderOrbits(camera->getView(), camera->getProjection(), cachedSatelliteStates,
				orbitCache, true);
	}

	ui->drawMenu(timeManager->getStringCurrentTime());
}

void Application::loadDataFromDatabase()
{
	auto updateResult = dataManager->updateTleData(backupPath);

	if (updateResult == DataManager::UpdateResult::SuccessFromNetwork ||
		updateResult == DataManager::UpdateResult::SuccessFromFile) {
		if (!dataManager->sortSatellitesIntoGroups()) {
			std::cerr << "Failed to sort satellites into groups" << std::endl;
			return;
		}
	}

	auto groups = dataManager->getAllGroupNames();
	std::cout << "Available groups: " << groups.size() << std::endl;
	for (const auto& group : groups) {
		auto satellites = dataManager->getSatellitesByGroup(group);
		groupSize[group] = satellites.size();
		std::cout << "Group " << group << ": " << satellites.size() << " satellites" << std::endl;
	}
}

bool Application::initializeManagers()
{
	// Создаём камеру после того как узнали размеры окна
	camera = new Camera(width, height);
	if (!camera) {
		std::cerr << "Failed to create camera" << std::endl;
		return false;
	}

	eventBus = std::make_unique<EventBus>();
	
	// Создание менеджера времени
	timeManager = std::make_shared<TimeManager>();

	std::filesystem::create_directories("data");
	std::filesystem::create_directories("data/backups");

	dataManager = std::make_shared<DataManager>(dbPath);
	if (!dataManager->initialize()) {
		std::cerr << "Failed to initialize DataManager!" << std::endl;
		return false;
	}

	satelliteManager = std::make_shared<SatelliteManager>(dataManager);
	if (!satelliteManager->initialize()) {
		std::cerr << "Failed to initialize SatelliteManager" << std::endl;
		return false;
	}

	satelliteRenderer = std::make_unique<SatelliteRenderer>(renderOptions);
	if (!satelliteRenderer->initialize()) {
		std::cerr << "Failed to init SatelliteRenderer!" << std::endl;
		return false;
	}

	dataManager->setEventBus(eventBus.get());
	satelliteManager->setEventBus(eventBus.get());

	{
		std::lock_guard<std::mutex> lock(dataMutex);
		cachedSatelliteStates.clear();
		orbitCache.clear();
	}

	return true;
}

float Application::calculateSatelliteSize()
{
	// Адаптивный размер спутников в зависимости от разрешения
	float baseSize = 4.0f;
	float cameraRadius = camera->getRadius();

	if (cameraRadius > 4.0f) {
		float maxR = 20.0f, minR = 3.0f;
		float maxSize = 3.0f, minSize = 1.0f;
		baseSize = maxSize + (cameraRadius - minR) * (minSize - maxSize) / (maxR - minR);
	}

	// Масштабирование в зависимости от разрешения
	float scaleFactor = std::min(width / 1920.0f, height / 1080.0f);
	return baseSize * scaleFactor;
}