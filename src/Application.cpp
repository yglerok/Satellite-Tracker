#include "Application.h"

#include <iostream>
#include <cmath>

Application::Application(const char* appTitle, int appWidth, int appHeight) :
	title(appTitle), width(appWidth), height(appHeight)
{
	view = glm::lookAt(
		glm::vec3(5.0f, 0.0f, 5.0f),	// ������� ������
		glm::vec3(0.0f, 0.0f, 0.0f),	// ����������� ������� (�����, � ������� ������� ������)
		glm::vec3(0.0f, 1.0f, 0.0f)		// ������ "�����" (������ (0, 1, 0))
	);

	projection = glm::perspective(
		glm::radians(45.0f),	// ���� ������ (FOV) (45-90 ����)
		float(width) / float(height),		// ����������� ������ (������ / ������)
		0.1f,					// ������� ��������� ��������� 
		100.0f					// ������� ��������� ���������
	);

	model = glm::mat4(1.0f);
}

bool Application::init()
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "ERROR [SDL] Can't initialize SDL!" << std::endl;
		return false;
	}

	window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_OPENGL);

	// ��������� ��������� openGL
	context = SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

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

	return true;
}

void Application::start()
{
	// �������� �������
	shaderProgram = Shader::create("res/shaders/earth.vert", "res/shaders/earth.frag");

	// �������� ������ �����
	earth = new Earth();

	// ��������� OpenGL
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	while (isRunning) {
		processInput();

		update();

		render();

		SDL_Delay(16);
	}
}

void Application::shutdown()
{
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
}

void Application::update()
{
	currentRotationPhi = glm::mix(currentRotationPhi, targetRotationPhi, rotationInterpolationSpeed);
	currentRotationTheta = glm::mix(currentRotationTheta, targetRotationTheta, rotationInterpolationSpeed);
	currentRadius = glm::mix(currentRadius, targetRadius, scaleInterpolationSpeed);

	float camX = cos(currentRotationTheta) * sin(currentRotationPhi) * currentRadius;
	float camZ = cos(currentRotationTheta) * cos(currentRotationPhi) * currentRadius;
	float camY = sin(currentRotationTheta) * currentRadius;

	view = glm::lookAt(
		glm::vec3(camX, camY, camZ),	
		glm::vec3(0.0f, 0.0f, 0.0f),	
		glm::vec3(0.0f, 1.0f, 0.0f)		
	);

	
	// ��������� ���������
	Shader::setFloat(shaderProgram, "ambientStrength", inputParams.ambientStrength);
	Shader::setFloat(shaderProgram, "specularStrength", inputParams.specularStrength);
	Shader::setVec3(shaderProgram, "objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
	Shader::setVec3(shaderProgram, "lightPos", glm::vec3(inputParams.lightPos[0], inputParams.lightPos[1], inputParams.lightPos[2]));
	Shader::setVec3(shaderProgram, "lightColor", glm::vec3(inputParams.lightColor[0], inputParams.lightColor[1], inputParams.lightColor[2]));

	Shader::setFloat(shaderProgram, "nightIntensity", inputParams.nightTextureIntensity);
}

void Application::render()
{
	// ������� �������
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// ��������� �����
	earth->render(model, view, projection, shaderProgram);


	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(width);
	io.DisplaySize.y = static_cast<float>(height);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();
	ImGui::Begin("Lightning settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::DragFloat("Ambient strength", &inputParams.ambientStrength, 0.001f, 0.0f, 0.3f);
	ImGui::DragFloat("Specular strength", &inputParams.specularStrength, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat3("Light position (x, y, z)", inputParams.lightPos, 0.1f, -15.0f, 15.0f);
	ImGui::ColorEdit3("Light color", inputParams.lightColor);

	ImGui::End();

	ImGui::Begin("Texture settings", 0, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::DragFloat("Night Intensity", &inputParams.nightTextureIntensity, 0.01f, 0.0f, 2.0f);
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(window);
}
