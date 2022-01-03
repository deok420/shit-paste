#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Box;

struct PlayersPreviousInfo_t {
	int hp = -1;
	int hpDifference = 0;
	float hpDiffTime = 0.f;

	int armor = -1;
	int armorDifference = 0;
	float armorDiffTime = 0.f;
};

class playeresp : public singleton <playeresp>
{
	int alpha = 255;

	void ImplementAlpha(Color& clr) {
		if ((int)clr.a() > alpha)
			clr.SetColor(clr.r(), clr.g(), clr.b(), alpha);
	}

public:
	int type = ENEMY;
	float esp_alpha_fade[65];
	int health[65];
	HPInfo hp_info[65];

	RECT bbox{ 0,0,0,0 };

	int32_t health1 = 75;

	PlayersPreviousInfo_t* previousInfo;

	void paint_traverse();
	void anotaone();
	void centerinds();
	void draw_box(player_t* m_entity, const Box& box);
	void keylist();
	void keylist1();
	void speclist();
	void evolveinds();
	void draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox);
	void draw_skeleton(player_t* e, Color color, matrix3x4_t matrix[MAXSTUDIOBONES]);
	bool draw_ammobar(player_t* m_entity, const Box& box);
	void draw_name(player_t* m_entity, const Box& box);
	void draw_weapon(player_t* m_entity, const Box& box, bool space);
	bool IsChoke();
	void draw_flags(player_t* e, const Box& box);
	void draw_multi_points(player_t* e);
};