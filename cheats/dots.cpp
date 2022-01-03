#include "dots.h"
#include "..\includes.hpp"
#include <vector>

std::vector<dot*> dots = { };

void dot::update() {
	auto opacity = 255.0f / 255.0f;

	m_pos += m_vel * (opacity);
}

void dot::draw() {
	int opacity = 255.0f * (255.0f / 255.0f);

	static auto list = ImGui::GetBackgroundDrawList();

	//list->AddRectFilled(ImVec2(0, 0), ImVec2(1920, 1080), ImGui::ColorConvertFloat4ToU32(ImVec4(0.f,0.f, 0.f, 80.f / 255.f)));

	list->AddCircleFilled(ImVec2(m_pos.x, m_pos.y), 2.f, ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 80.f / 255.f)));
	//list->AddRectFilled(ImVec2(m_pos.x - 2, m_pos.y - 2), ImVec2(m_pos.x + 2, m_pos.y + 2), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, opacity / 255.f)));

	auto t = std::find(dots.begin(), dots.end(), this);
	if (t == dots.end()) {
		return;
	}

	for (auto i = t; i != dots.end(); i++) {
		if ((*i) == this) continue;

		auto dist = (m_pos - (*i)->m_pos).Length();

		if (dist < 128) {
			int alpha = opacity * (dist / 128);
			list->AddLine(ImVec2(m_pos.x - 1, m_pos.y - 2), ImVec2((*i)->m_pos.x - 2, (*i)->m_pos.y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, alpha / 255.f)));
		}
	}
}

void dot_draw() {
	struct screen_size {
		int x, y;
	}; screen_size sc;

	m_engine()->GetScreenSize(sc.x, sc.y);

	int s = rand() % 12;

	if (s == 0) {
		dots.push_back(new dot(Vector(rand() % (int)sc.x, -16, 0), Vector((rand() % 7) - 3, rand() % 3 + 1, 0)));
	}
	else if (s == 1) {
		dots.push_back(new dot(Vector(rand() % (int)sc.x, (int)sc.y + 16, 0), Vector((rand() % 7) - 3, -1 * (rand() % 3 + 1), 0)));
	}
	else if (s == 2) {
		dots.push_back(new dot(Vector(-16, rand() % (int)sc.y, 0), Vector(rand() % 3 + 1, (rand() % 7) - 3, 0)));
	}
	else if (s == 3) {
		dots.push_back(new dot(Vector((int)sc.x + 16, rand() % (int)sc.y, 0), Vector(-1 * (rand() % 3 + 1, 0), (rand() % 7) - 3, 0)));
	}

	for (auto i = dots.begin(); i < dots.end();) {
		if ((*i)->m_pos.y < -20 || (*i)->m_pos.y > sc.y + 20 || (*i)->m_pos.x < -20 || (*i)->m_pos.x > sc.x + 20) {
			delete (*i);
			i = dots.erase(i);
		}
		else {
			(*i)->update();
			i++;
		}
	}

	

	for (auto i = dots.begin(); i < dots.end(); i++) {
		(*i)->draw();
	}
}

void dot_destroy() {
	for (auto i = dots.begin(); i < dots.end(); i++) {
		delete (*i);
	}

	dots.clear();
}