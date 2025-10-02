#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl3.h>

#include <SDL3/SDL.h>

#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>

#include "data/DataManager.h"
#include "satellitesManagement/SatelliteManager.h"

struct RenderOptions {
	bool areSatellitesVisible = true;
	bool areOrbitsVisible = true;
	float satellitesColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float orbitsColor[4] = { 1.0f, 1.0f, 1.0f, 0.5f };
};

class UI
{
public:
	UI() = delete;
	UI(const int& width, const int& height, std::shared_ptr<DataManager> dataMngr, 
		std::shared_ptr<SatelliteManager> satelliteMngr, RenderOptions& options) :
		windowWidth(width), windowHeight(height), dataManager(dataMngr), 
		satelliteManager(satelliteMngr), renderOptions(options) { };
	~UI() = default;

	bool initialize(SDL_Window* window, SDL_GLContext* context);
	void drawMenu(const std::string& timeString);

private:
	int windowWidth, windowHeight;
	std::unordered_map<std::string, bool> filters;
	std::shared_ptr<DataManager> dataManager;
	std::shared_ptr<SatelliteManager> satelliteManager;

	RenderOptions& renderOptions;

	void drawTimeBlock();
	void drawForcedUpdateBlock();
	void drawFilters();
};