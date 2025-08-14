#pragma once

#include <glad/glad.h>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>

#include <string>

#include "render/Earth.h"
#include "render/Shaders.h"

static struct MouseState {
	bool isPressed = false;
	float prevX = 0.0f, prevY = 0.0f;
};

struct InputParameters {
	float ambientStrength = 0.05f, specularStrength = 0.1f;
	float lightPos[3] = { 2.0f, 3.0f, 3.0f },
		lightColor[3] = { 1.0f, 1.0f, 1.0f };
	float nightTextureIntensity = 0.3f;
};

class Application
{
public:
	Application(const char* appTitle, int appWidth, int appHeight);
	~Application() = default;
	
	bool init();
	void start();
	void shutdown();

private:
	std::string title;
	int width, height;
	bool isRunning = true;

	void processInput();
	void update();
	void render();

	SDL_Window* window = nullptr;
	SDL_GLContext context;
	GLuint shaderProgram = 0;
	Earth* earth = nullptr;

	float currentRotationPhi = 0.0f;    // Текущий угол
	float currentRotationTheta = 0.0f;
	float targetRotationPhi = 0.0f;    // Целевой угол
	float targetRotationTheta = 0.0f;
	const float mouseSensitivity = 0.01f;
	const float rotationInterpolationSpeed = 0.1f;

	float targetRadius = 5.0f;
	float currentRadius = 5.0f;
	const float scaleInterpolationSpeed = 0.05f;

	MouseState mouseState;
	InputParameters inputParams;

	// Матрица вида
	glm::mat4 view;
	// Задает перспективную проекию (имитирует человеческое зрение)
	// Преобразует 3D-координаты в 2D-экранные с учётом перспективы 
	// (дальние объекты выглядят меньше)
	glm::mat4 projection;
	// Матрица модели (вращение земли)
	glm::mat4 model;
};