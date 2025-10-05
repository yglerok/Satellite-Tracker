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
	float satellitesColor[4] = { 1.0f, 0.0f, 0.0f, 0.664f };
	float orbitsColor[4] = { 1.0f, 1.0f, 1.0f, 0.2f };
};

struct MouseState {
	bool isPressed = false;
	float prevX = 0.0f, prevY = 0.0f;
	bool isOnMenuScrollArea = false;
};

struct FilterGroup {
	std::string name;
	bool isEnabled;
	std::map<std::string, bool> subgroups;
};

class UI
{
public:
	UI() = delete;
	UI(const int& width, const int& height, std::shared_ptr<DataManager> dataMngr, 
		std::shared_ptr<SatelliteManager> satelliteMngr, RenderOptions& options, MouseState& state, 
		std::unordered_map<std::string, int>& satsInGroups) :
		windowWidth(width), windowHeight(height), dataManager(dataMngr), 
		satelliteManager(satelliteMngr), renderOptions(options), mouseState(state), groupSize(satsInGroups) { };
	~UI() = default;

	bool initialize(SDL_Window* window, SDL_GLContext* context);
	void drawMenu(const std::string& timeString);
	void drawLoadingWindow();

private:
	int windowWidth, windowHeight;
	std::map<std::string, FilterGroup> filtersByName;
	std::map<std::string, FilterGroup> filtersByOrbitType;
	std::map<std::string, FilterGroup> noFilters;
	
	std::unordered_map<std::string, int>& groupSize;

	std::vector<std::string> orbitTypes = { "Geostationary", "Low Earth Orbit",
		"Medium Earth Orbit", "High Earth Orbit", "Polar Orbit",
		"Very Low Earth Orbit", "Highly Elliptical Orbit", "Geosynchronous Orbit",
		"Other" };
	std::shared_ptr<DataManager> dataManager;
	std::shared_ptr<SatelliteManager> satelliteManager;

	RenderOptions& renderOptions;
	MouseState& mouseState;

	void initializeFilterGroups();

	void drawTimeBlock(const std::string& timeString);
	void drawOptionsBlock();
	void drawFiltersBlock();
	void drawColorBlock();
};