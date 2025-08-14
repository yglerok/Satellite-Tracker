#include <iostream>
#include <string>

#include "Application.h"

const int WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 780;
std::string WINDOW_TITLE = "Satellite Tracker";

int main()
{
	Application app(WINDOW_TITLE.c_str(), WINDOW_WIDTH, WINDOW_HEIGHT);
	
	if (!app.init()) { // ������������� ������ : SDL, Glad, ImGui
		std::cout << "App initialization failed!" << std::endl;
		return -1;
	}
	
	app.start(); // �������� ����
	
	app.shutdown(); // ������� ��������
	
	return 0;
}
