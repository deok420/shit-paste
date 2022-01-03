#include "c_gui.hpp"


std::map<ImGuiID, float> tab_alpha;
bool c_gui::tab(const char* icon, bool active, ImVec2 size_arg) {
	IM_GUI;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(icon);
	const ImVec2 label_size = CalcTextSize(icon, NULL, true);
	ImGuiButtonFlags flags = 0;
	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
	float* alpha = &tab_alpha[id];

	if (*alpha >= GetStyle().Alpha) {
		*alpha = GetStyle().Alpha;
	}

	if (*alpha <= 0.f) {
		*alpha = 0.f;
	}

	if (active) {
		if (*alpha <= GetStyle().Alpha) {
			*alpha += ImGui::GetIO().DeltaTime * 6;
		}

	}

	if (!active) {
		if (*alpha > 0.f) {
			*alpha -= ImGui::GetIO().DeltaTime * 6;
		}
	}

	// Render
	ImU32 col = ImColor(0, 0, 0, 0);
	ImU32 t_col = ImColor(255, 255, 255, 180);

	if (active) {
		col = GetColorU32(ImVec4(52 / 255.f, 54 / 255.f, 61 / 255.f, *alpha));
		t_col = GetColorU32(ImVec4(1.f, 1.f, 1.f, *alpha));
	}
	else if (!active) {
		col = GetColorU32(ImVec4(52 / 255.f, 54 / 255.f, 61 / 255.f, *alpha));
		t_col = GetColorU32(ImVec4(1.f, 1.f, 1.f, *alpha));
	}

	RenderFrame(bb.Min + ImVec2(5, 5), bb.Max - ImVec2(5, 5), col, true, 5.f);

	window->DrawList->AddText(c_menu::get().tab_icon, 18.f, bb.Min + ImVec2(16, 15), ImColor(255, 255, 255, 180), icon);

	window->DrawList->AddText(c_menu::get().tab_icon, 18.f, bb.Min + ImVec2(16, 15), t_col, icon);
	return pressed;
}
std::map<ImGuiID, float> sub_alpha;
bool c_gui::subtab(const char* label, bool active, ImVec2 size_arg) {
	IM_GUI;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImGuiButtonFlags flags = 0;

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
	float* alpha = &sub_alpha[id];

	if (*alpha >= GetStyle().Alpha) {
		*alpha = GetStyle().Alpha;
	}

	if (*alpha <= 0.f) {
		*alpha = 0.f;
	}

	if (active) {
		if (*alpha <= GetStyle().Alpha) {
			*alpha += ImGui::GetIO().DeltaTime * 6;
		}

	}

	if (!active) {
		if (*alpha > 0.f) {
			*alpha -= ImGui::GetIO().DeltaTime * 6;
		}
	}

	// Render
	ImU32 col = ImColor(0, 0, 0, 0);
	ImU32 c_col = ImColor(255, 255, 255, 180);
	ImU32 t_col = ImColor(255, 255, 255, 180);

	if (active) {
		col = GetColorU32(ImVec4(44 / 255.f, 43 / 255.f, 48 / 255.f, *alpha));
		c_col = GetColorU32(ImVec4(0 / 255.f, 128 / 255.f, 0.f, *alpha));
		t_col = GetColorU32(ImVec4(1.f, 1.f, 1.f, *alpha));
	}
	else if (!active) {
		col = GetColorU32(ImVec4(44 / 255.f, 43 / 255.f, 48 / 255.f, *alpha));
		c_col = GetColorU32(ImVec4(0 / 255.f, 128 / 255.f, 0.f, *alpha));
		t_col = GetColorU32(ImVec4(1.f, 1.f, 1.f, *alpha));
	}

	RenderFrame(bb.Min, bb.Max, col, true, 4.f);
	

	window->DrawList->AddCircleFilled(bb.Min + ImVec2(15, 17.5f), 3.f, ImColor(255, 255, 255, 180), 360);
	window->DrawList->AddCircleFilled(bb.Min + ImVec2(15, 17.5f), 3.5f, c_col, 360);


	window->DrawList->AddText(bb.Min + ImVec2(30, 10), ImColor(255, 255, 255, 180), label);

	window->DrawList->AddText(bb.Min + ImVec2(30, 10), t_col, label);
	return pressed;
}