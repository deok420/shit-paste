#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"
struct m_indicator
{
	std::string m_text;
	Color m_color;

	m_indicator(const char* text, Color color) :
		m_text(text), m_color(color)
	{

	}
	m_indicator(std::string text, Color color) :
		m_text(text), m_color(color)
	{

	}

};

struct damage_indicator_t
{
	int dmg;
	bool initializes;
	float earse_time;
	float last_update;
	player_t* player;
	Vector Position;
};

extern std::vector<damage_indicator_t> dmg_indicator;

class otheresp : public singleton< otheresp >
{
public:
	void draw_arc(int x, int y, float r1, float r2, int s, int d, Color m_clr);
	void penetration_reticle();
	void indicators();
	void circlefake();
	void fake();
	void draw_velocity();
	void draw_indicators();
	void hitmarker_paint();
	void damage_marker_paint();
	void spread_crosshair(LPDIRECT3DDEVICE9 device);
	void automatic_peek_indicator();

	//void draw_arc(int x, int y, int radius, int startangle, int percent, int thickness, Color color);

	void grenade();

	void DrawMolotov();

	void DrawDamageIndicator();

	struct Hitmarker
	{
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		Vector point = ZERO;
	} hitmarker;

	struct Damage_marker
	{
		Vector position = ZERO;
		float positiony = FLT_MIN;
		float hurt_time = FLT_MIN;
		Color hurt_color = Color::White;
		int damage = -1;
		int hitgroup = -1;

		void reset()
		{
			position.Zero();
			hurt_time = FLT_MIN;
			hurt_color = g_cfg.esp.damage_marker_color;
			damage = -1;
			hitgroup = -1;
		}
	} damage_marker[65];
	std::vector<m_indicator> m_indicators;
};