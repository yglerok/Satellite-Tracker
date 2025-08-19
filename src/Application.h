#pragma once

#include <glad/glad.h>

#include <SDL3/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>

#include <string>
#include <chrono>

#include "render/Earth.h"
#include "render/Shaders.h"
#include "render/Sun.h"
#include "Camera.h"

static struct MouseState {
	bool isPressed = false;
	float prevX = 0.0f, prevY = 0.0f;
};

struct InputParameters {
	float ambientStrength = 0.05f, specularStrength = 0.1f;
	float //lightPos[3] = { 2.0f, 3.0f, 3.0f },
		lightColor[3] = { 1.0f, 1.0f, 1.0f };
	float nightTextureIntensity = 0.4f;
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
	const int FPS = 60;
	
	void processInput();
	void update(double dt);
	void render(double alpha);

	SDL_Window* window = nullptr;
	SDL_GLContext context;
	GLuint shaderProgram = 0;
	Camera* camera;
	Earth* earth = nullptr;
	Sun sun;

	const float mouseSensitivity = 0.01f;

	MouseState mouseState;
	InputParameters inputParams;
};