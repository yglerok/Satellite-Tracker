#include "UI.h"

bool UI::initialize(SDL_Window* window, SDL_GLContext* context)
{
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

	// Извлечение фильтров групп
	for (const auto& groupName : dataManager->getAllGroupNames()) {
		filters.emplace(groupName, true);
	}

	return true;
}

void UI::drawMenu(const std::string& timeString)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(windowWidth);
	io.DisplaySize.y = static_cast<float>(windowHeight);

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
	ImGui::Text(timeString.c_str());
	ImGui::SeparatorText("Options");
	ImGui::Checkbox("Show satellites", &renderOptions.areSatellitesVisible);
	ImGui::SameLine();
	ImGui::Checkbox("Show orbits", &renderOptions.areOrbitsVisible);
	/*if (ImGui::Button("Forced update from network")) {
		dataManager->forceUpdateTleFromNetwork();
	}
	if (ImGui::Button("Sort satellites")) {
		dataManager->sortSatellitesIntoGroups();
	}*/

	if (ImGui::CollapsingHeader("Filters")) {
		//ImGui::SeparatorText("Info");
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

	if (ImGui::CollapsingHeader("Color options")) {
		ImGui::Text("Here you can choose colors for satellites and orbits.");
		/*ImGui::Text("Satellites color:");
		ImGui::SameLine();*/
		ImGui::ColorEdit4("Satellites color", renderOptions.satellitesColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4("Orbits color", renderOptions.orbitsColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
