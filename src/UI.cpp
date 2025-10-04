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

	// Инициализируем иерархические фильтры
	initializeFilterGroups();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = static_cast<float>(windowWidth);
	io.DisplaySize.y = static_cast<float>(windowHeight);

	return true;
}

void UI::drawMenu(const std::string& timeString)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	//ImGui::ShowDemoWindow();

	ImGui::Begin("Menu", 0, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SeparatorText("Time");
	ImGui::Text(timeString.c_str());
	ImGui::SeparatorText("Options");
	ImGui::Checkbox("Show satellites", &renderOptions.areSatellitesVisible);
	ImGui::SameLine();
	ImGui::Checkbox("Show orbits", &renderOptions.areOrbitsVisible);
	/*if (ImGui::Button("Forced update from network")) {
		dataManager->forceUpdateTleFromNetwork();
	}*/
	if (ImGui::Button("Sort satellites")) {
		dataManager->sortSatellitesIntoGroups();
	}

	if (ImGui::CollapsingHeader("Filters")) {
		//ImGui::SeparatorText("Info");
		ImGui::Text("Here you can choose how to sort satellites\n(by name or by orbit type).");

		static int filterType = 0; // 0 - by name, 1 - by orbit type
		ImGui::RadioButton("By name", &filterType, 0);
		ImGui::RadioButton("By orbit type", &filterType, 1);

		auto& currentFilters = (filterType == 1) ? filtersByOrbitType : filtersByName;

		if (ImGui::Button("Select all")) {
			for (auto& [groupName, group] : currentFilters) {
				group.isEnabled = true;
				for (auto& [subgroup, state] : group.subgroups)
					state = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Unselect all")) {
			for (auto& [groupName, group] : currentFilters) {
				group.isEnabled = false;
				for (auto& [subgroup, state] : group.subgroups)
					state = false;
			}
		}

		ImGui::BeginChild("Filters", ImVec2(ImGui::GetContentRegionAvail().x, 300),
			ImGuiChildFlags_None, ImGuiWindowFlags_None);

		for (auto& [groupName, group] : currentFilters) {
			if (ImGui::Checkbox(groupName.c_str(), &group.isEnabled)) {
				for (auto& [subgroup, state] : group.subgroups)
					state = group.isEnabled;
			}

			// Отступ
			ImGui::Indent(20.0f);

			if (group.isEnabled) {
				for (auto& [subgroup, state] : group.subgroups)
					ImGui::Checkbox(subgroup.c_str(), &state);
			}
			else {
				ImGui::TextDisabled("(%d subgroups)", group.subgroups.size());
			}

			ImGui::Unindent(20.0f);
			ImGui::Spacing();
		}

		ImGui::EndChild();

		mouseState.isOnMenuScrollArea = (ImGui::IsItemHovered()) ? true : false;

		std::unordered_map<std::string, bool> flatFilters;

		for (const auto& [groupName, group] : currentFilters) {
			if (group.subgroups.empty()) {
				flatFilters[groupName] = group.isEnabled ? true : false;
				continue;
			}

			for (const auto& [subgroup, state] : group.subgroups) {
				flatFilters[subgroup] = state ? true : false;
			}
		}

		satelliteManager->setGroupFilters(flatFilters);
	}

	if (ImGui::CollapsingHeader("Color options")) {
		ImGui::Text("Here you can choose colors for satellites\nand orbits.");
		ImGui::ColorEdit4("Satellites color", renderOptions.satellitesColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
		ImGui::ColorEdit4("Orbits color", renderOptions.orbitsColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::drawLoadingWindow()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	// Делаем окно более заметным
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::Begin("Loading", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground);

	// Центрируем текст с большим шрифтом
	ImGui::SetCursorPos(ImVec2(
		(ImGui::GetIO().DisplaySize.x - 200) * 0.5f,
		(ImGui::GetIO().DisplaySize.y - 50) * 0.5f
	));

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "LOADING SATELLITES DATA...");

	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::initializeFilterGroups()
{
	// Крупные созвездия
	filtersByName["Constellations"] = { "Constellations", true, {
		{"Starlink", true}, {"OneWeb", true}, {"Iridium", true},
		{"Globalstar", true}, {"ORBCOMM", true}, {"Planet Labs", true}, {"Spire", true}
	} };

	// Навигационные системы
	filtersByName["Navigation"] = { "Navigation", true, {
		{"GPS", true}, {"GLONASS", true}, {"Galileo", true},
		{"BeiDou", true}, {"IRNSS", true}, {"QZSS", true}
	} };

	// Коммуникационные операторы
	filtersByName["Communication"] = { "Communication", true, {
		{"Intelsat", true}, {"SES", true}, {"Eutelsat", true},
		{"Telesat", true}, {"Inmarsat", true}, {"Thuraya", true}
	} };

	// Научные и исследовательские
	filtersByName["Scientific"] = { "Scientific", true, {
		{"NASA", true}, {"ESA", true}, {"Hubble Space Telescope", true},
		{"James Webb Telescope", true}, {"Planetary Science", true}, {"Astronomy", true}
	} };

	// Метеорологические
	filtersByName["Weather"] = { "Weather", true, {
		{"NOAA", true}, {"GOES", true}, {"Meteosat", true},
		{"Fengyun", true}, {"Electro-L", true}
	} };

	// Военные
	filtersByName["Military"] = { "Military", true, {
		{"US Military", true}, {"Russian Military", true},
		{"Reconnaissance", true}, {"Early Warning", true}
	} };

	// Мониторинг Земли
	filtersByName["Earth Observation"] = { "Earth Observation", true, {
		{"Landsat", true}, {"Sentinel", true}, {"SPOT", true},
		{"High Resolution Imaging", true}
	} };

	// По странам
	/*filtersByName["Countries"] = { "Countries", true, {
		{"Russian", true}, {"Chinese", true}, {"Indian", true},
		{"Japanese", true}, {"European", true}
	} };*/

	// Специализированные
	filtersByName["Specialized"] = { "Specialized", true, {
		{"Amateur Radio", true}, {"CubeSat", true}, {"Technology Demo", true},
		{"Space Station Related", true}, {"Debris/Rocket Body", true}
	} };

	// === ОРБИТАЛЬНЫЕ ГРУППЫ ===
	/*filtersByOrbitType["Orbits"] = { "Orbits", true, {
		{"Geostationary", true}, {"Geosynchronous Orbit", true},
		{"Low Earth Orbit", true}, {"Very Low Earth Orbit", true},
		{"Polar Orbit", true}, {"Medium Earth Orbit", true},
		{"Highly Elliptical Orbit", true}, {"High Earth Orbit", true},
		{"Other", true}
	} };*/

	filtersByOrbitType["Low"] = { "Low", true, {
		{"Very Low Earth Orbit", true}, {"Low Earth Orbit", true}, 
		{"Polar Orbit", true}
	} };

	filtersByOrbitType["Medium Earth Orbit"] = { "Medium Earth Orbit", true };

	filtersByOrbitType["High"] = { "High", true, {
		{"Geostationary", true}, {"Geosynchronous Orbit", true},
		{"Highly Elliptical Orbit", true}, {"High Earth Orbit", true}
	} };

	filtersByOrbitType["Other"] = { "Other", true };
}
