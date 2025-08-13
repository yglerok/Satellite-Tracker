#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render/Earth.h"
#include "render/Shaders.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>

const int WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 780;

int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "ERROR::Can't initialize SDL!" << std::endl;
	}

	SDL_Window* window = SDL_CreateWindow("Satellite Tracker", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	gladLoadGL();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplOpenGL3_Init();
	ImGui_ImplSDL3_InitForOpenGL(window, context);

	// Загрузка шейдера
	GLuint shader = Shader::create("res/shaders/earth.vert", "res/shaders/earth.frag");

	// Создание модели Земли
	Earth* earth = new Earth();

	glm::mat4 view = glm::lookAt(
		glm::vec3(5.0f, 0.0f, 5.0f),	// позиция камеры
		glm::vec3(0.0f, 0.0f, 0.0f),	// направление взгляда (точка, в которую смотрит камера)
		glm::vec3(0.0f, 1.0f, 0.0f)		// вектор "вверх" (обычно (0, 1, 0))
	);
	// Задает перспективную проекию (имитирует человеческое зрение)
	// Преобразует 3D-координаты в 2D-экранные с учётом перспективы 
	// (дальние объекты выглядят меньше)
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f),	// угол обзора (FOV) (45-90 град)
		1024.0f / 780.0f,		// соотношение сторон (ширина / высота)
		0.1f,					// ближняя плоскость отсечения 
		100.0f					// дальняя плоскость отсечения
	);

	// Матрица модели (вращение земли)
	glm::mat4 model = glm::mat4(1.0f);

	//float rotationTheta = 0.0f, rotationPhi = 0.0f;
	float currentRotationPhi = 0.0f;    // Текущий угол
	float currentRotationTheta = 0.0f;
	float targetRotationPhi = 0.0f;    // Целевой угол
	float targetRotationTheta = 0.0f;
	const float mouseSensitivity = 0.01f;
	const float rotationInterpolationSpeed = 0.1f;

	float targetRadius = 5.0f;
	float currentRadius = 5.0f;
	const float scaleInterpolationSpeed = 0.01f;

	static struct MouseState {
		bool isPressed = false;
		float prevX = 0.0f, prevY = 0.0f;
	} mouseState;

	// Настройка OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


	bool isRunning = true;
	while (isRunning) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				isRunning = false;
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				targetRadius -= event.wheel.y * 0.5;
				targetRadius = glm::clamp(targetRadius, 2.0f, 15.0f);
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

				targetRotationPhi -= dx * mouseSensitivity;
				targetRotationTheta += dy * mouseSensitivity;

				targetRotationTheta = glm::clamp(targetRotationTheta, -1.4f, 1.4f);

				mouseState.prevX = event.motion.x;
				mouseState.prevY = event.motion.y;
				break;
			}
			case SDL_EVENT_KEY_DOWN:
				if (event.key.scancode == SDL_SCANCODE_R) {
					targetRotationTheta = 0.0f;
					targetRotationPhi = 0.0f;
				}
				break;
			default:
				break;
			}
				
		}

		// Очистка буферов
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentRotationPhi = glm::mix(currentRotationPhi, targetRotationPhi, rotationInterpolationSpeed);
		currentRotationTheta = glm::mix(currentRotationTheta, targetRotationTheta, rotationInterpolationSpeed);
		currentRadius = glm::mix(currentRadius, targetRadius, scaleInterpolationSpeed);

		float camX = cos(currentRotationTheta) * sin(currentRotationPhi) * currentRadius;
		float camZ = cos(currentRotationTheta) * cos(currentRotationPhi) * currentRadius;
		float camY = sin(currentRotationTheta) * currentRadius;
		

		view = glm::lookAt(
			glm::vec3(camX, camY, camZ),	// позиция камеры
			glm::vec3(0.0f, 0.0f, 0.0f),	// направление взгляда (точка, в которую смотрит камера)
			glm::vec3(0.0f, 1.0f, 0.0f)		// вектор "вверх" (обычно (0, 1, 0))
		);


		static float ambientStrength = 0.05f, specularStrength = 0.1f;
		static float lightPos[3] = { 2.0f, 3.0f, 3.0f },
			lightColor[3] = {1.0f, 1.0f, 1.0f};
		static float nightTextureIntensity = 0.3f;

		// Настройка освещения
		Shader::setFloat(shader, "ambientStrength", ambientStrength);
		Shader::setFloat(shader, "specularStrength", specularStrength);
		Shader::setVec3(shader, "objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
		Shader::setVec3(shader, "lightPos", glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
		Shader::setVec3(shader, "lightColor", glm::vec3(lightColor[0], lightColor[1], lightColor[2]));

		Shader::setFloat(shader, "nightIntensity", nightTextureIntensity);

		// Отрисовка Земли
		earth->render(model, view, projection, shader);


		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = static_cast<float>(WINDOW_WIDTH);
		io.DisplaySize.y = static_cast<float>(WINDOW_HEIGHT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();
		ImGui::Begin("Lightning settings", 0, ImGuiWindowFlags_AlwaysAutoResize);
		
		ImGui::DragFloat("Ambient strength", &ambientStrength, 0.001f, 0.0f, 0.3f);
		ImGui::DragFloat("Specular strength", &specularStrength, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat3("Light position (x, y, z)", lightPos, 0.1f, -15.0f, 15.0f);
		ImGui::ColorEdit3("Light color", lightColor);

		ImGui::End();

		ImGui::Begin("Texture settings", 0, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::DragFloat("Night Intensity", &nightTextureIntensity, 0.01f, 0.0f, 2.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(window);
	}

	delete earth;
	glDeleteProgram(shader);
	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
