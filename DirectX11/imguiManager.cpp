#include "imguiManager.h"
#include "imgui/imgui.h"

imguiManager::imguiManager()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
}

imguiManager::~imguiManager()
{
	ImGui::DestroyContext();
}
