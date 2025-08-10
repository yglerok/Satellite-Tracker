#include <glad/glad.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "render/Earth.h"
#include "render/Shaders.h"


int main()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "ERROR::Can't initialize SDL!" << std::endl;
	}

	SDL_Window* window = SDL_CreateWindow("Satellite Tracker", 1024, 780, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	
	gladLoadGL();

	// Загрузка шейдера
	GLuint shader = Shader::create("res/shaders/earth.vert", "res/shaders/earth.frag");

	// Создание модели Земли
	Earth* earth = new Earth();

	// Настройка OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


	bool isRunning = true;
	while (isRunning) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT)
				isRunning = false;
		}

		// Очистка буферов
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Матрицы камеры
		
		// Задает положение и ориентацию камеры в мире
		// Преобразует мировые координаты в координаты камеры
		// (чтобы объекты двигались относительно камеры)
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 5.0f),	// позиция камеры
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

		// Отрисовка Земли
		earth->render(view, projection, shader);

		SDL_GL_SwapWindow(window);
	}

	delete earth;
	glDeleteProgram(shader);
	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
