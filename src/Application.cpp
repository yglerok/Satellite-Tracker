#include "Application.h"

#include <iostream>
#include <cmath>
#include <filesystem>

Application::Application(const char* appTitle, int appWidth, int appHeight) :
	title(appTitle), width(appWidth), height(appHeight)
{
	camera = new Camera(width, height);
	if (!camera) {
		std::cerr << "Failed to create camera" << std::endl;
	}
}

bool Application::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "ERROR [SDL] Can't initialize SDL!" << std::endl;
		return false;
	}
	
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
	
	if (!IMGUI_CHECKVERSION()) {
		std::cerr << "ERROR [ImGui] In IMGUI_CHECKVERSION()!" << std::endl;
		return false;
	}
	ImGui::CreateContext();
	if (!ImGui_ImplOpenGL3_Init()) {
		std::cerr << "ERROR [ImGui] Can't init ImplOpenGL3!" << std::endl;
		return false;
	}
	if (!ImGui_ImplSDL3_InitForOpenGL(window, context)) {
		std::cerr << "ERROR [ImGui] Can't init ImplSDL3 for OpenGL!" << std::endl;
		return false;
	}

	if (!initializeManagers()) {
		std::cerr << "Failed to init App!" << std::endl;
		return false;
	}

	if (!satelliteRenderer.initialize()) {
		std::cerr << "Failed to init satelliteRenderer!" << std::endl;
		return false;
	}

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

	auto previousTime = std::chrono::steady_clock::now();
	double lag = 0.0;
	constexpr double fixed_dt = 1.0 / 60.0; // Фиксированный шаг для обновления физики

	SDL_GL_SetSwapInterval(1); // Включаем vsync

	while (isRunning) {
		auto currentTime = std::chrono::steady_clock::now();
		auto elapsedTime = currentTime - previousTime;
		previousTime = currentTime;
		lag += std::chrono::duration<double>(elapsedTime).count();

		processInput();

		// Фиксированное обновление физики (максимум 5 раз за кадр)
		int updateCount = 0;
		while (lag >= fixed_dt && updateCount < 5) {
			update(fixed_dt);
			lag -= fixed_dt;
			updateCount++;
		}
		
		// Интерполяция для более плавного рендера
		double alpha = lag / fixed_dt;
		render(alpha);

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
		case SDL_EVENT_MOUSE_WHEEL:
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
	static double lastSatelliteCalculation = 0;

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
				satelliteManager->update(timeManager->getCurrentJulianDate(), 2.0, 100);//

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

void Application::render(double alpha)
{
	// Проверка ДО очистки
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL error before clear: " << err << std::endl;
	}

	// Очистка буферов
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->render(alpha);

	// Отрисовка Земли
	earth->render(camera->getView(), camera->getProjection(), shaderProgram);

	// Вычисление размера спутников в зависимости от отдаления камеры (используем линейную интерполяцию)
	float satelliteSize = 5.0f + (camera->getRadius() - 3.0f) * (0.5f - 5.0f) / (20.0f - 3.0f);
	satelliteRenderer.setSatelliteSize(satelliteSize);

	{
		std::lock_guard<std::mutex> lock(dataMutex);
		satelliteRenderer.renderSatellites(camera->getView(), camera->getProjection(), cachedSatelliteStates, true);
		satelliteRenderer.renderOrbits(camera->getView(), camera->getProjection(), cachedSatelliteStates, 
			orbitCache, true);
	}

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(width);
	io.DisplaySize.y = static_cast<float>(height);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
	//ImGui::Begin("Lightning settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

	//ImGui::DragFloat("Ambient strength", &inputParams.ambientStrength, 0.001f, 0.0f, 0.3f);
	//ImGui::DragFloat("Specular strength", &inputParams.specularStrength, 0.01f, 0.0f, 1.0f);
	////ImGui::DragFloat3("Light position (x, y, z)", inputParams.lightPos, 0.1f, -15.0f, 15.0f);
	//ImGui::ColorEdit3("Light color", inputParams.lightColor);

	//ImGui::End();

	ImGui::Begin("Menu", 0, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SeparatorText("Time");
	ImGui::Text(timeManager->getStringCurrentTime().c_str());

	if (ImGui::CollapsingHeader("Filters")) {
		//ImGui::SeparatorText("Info");
		ImGui::Text("...");
		ImGui::Text("Displaing groups:");
		ImGui::SameLine();
		if (ImGui::Button("Select all")) {
			for (auto& [filter, state] : filters)
				state = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Unselect all")) {
			for (auto& [filter, state] : filters)
				state = false;
		}
		for (auto& [filter, state] : filters) {
			ImGui::Checkbox(filter.c_str(), &state);
		}

		satelliteManager->setGroupFilters(filters);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::loadDataFromDatabase(const std::string& backupPath)
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
		std::cout << "Group " << group << ": " << satellites.size() << " satellites" << std::endl;
	}
}

bool Application::initializeManagers()
{
	eventBus = std::make_unique<EventBus>();
	
	// Создание менеджера времени
	timeManager = std::make_shared<TimeManager>();

	std::filesystem::create_directories("data");
	std::filesystem::create_directories("data/backups");

	const std::string dbPath = "data/satellites.db";
	const std::string backupPath = "data/backups/celestrak_backup.tle";

	dataManager = std::make_shared<DataManager>(dbPath);
	if (!dataManager->initialize()) {
		std::cerr << "Failed to initialize DataManager!" << std::endl;
		return false;
	}

	satelliteManager = std::make_unique<SatelliteManager>(dataManager);
	if (!satelliteManager->initialize()) {
		std::cerr << "Failed to initialize SatelliteManager" << std::endl;
		return false;
	}

	dataManager->setEventBus(eventBus.get());
	satelliteManager->setEventBus(eventBus.get());

	// Загрузка и сортировка спутников
	loadDataFromDatabase(backupPath);

	// Извлечение фильтров групп
	for (const auto& groupName : dataManager->getAllGroupNames()) {
		filters.emplace(groupName, true);
	}

	{
		std::lock_guard<std::mutex> lock(dataMutex);
		cachedSatelliteStates.clear();
		orbitCache.clear();
	}

	return true;
}
