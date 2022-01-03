#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <map>

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../cheats/menu.h"

#define IM_GUI using namespace ImGui;

class c_gui
{
public:
	//Window tabs render
	bool tab(const char* icon, bool active, ImVec2 size);

	//Window sub tabs render
	bool subtab(const char* label, bool active, ImVec2 size);
};

