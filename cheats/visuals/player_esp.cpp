// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "player_esp.h"
#include "..\misc\misc.h"
#include "..\ragebot\aim.h"
#include "dormant_esp.h"

class RadarPlayer_t
{
public:
	Vector pos; //0x0000
	Vector angle; //0x000C
	Vector spotted_map_angle_related; //0x0018
	DWORD tab_related; //0x0024
	char pad_0x0028[0xC]; //0x0028
	float spotted_time; //0x0034
	float spotted_fraction; //0x0038
	float time; //0x003C
	char pad_0x0040[0x4]; //0x0040
	__int32 player_index; //0x0044
	__int32 entity_index; //0x0048
	char pad_0x004C[0x4]; //0x004C
	__int32 health; //0x0050
	char name[32]; //0x785888
	char pad_0x0074[0x75]; //0x0074
	unsigned char spotted; //0x00E9
	char pad_0x00EA[0x8A]; //0x00EA
};

class CCSGO_HudRadar
{
public:
	char pad_0x0000[0x14C];
	RadarPlayer_t radar_info[65];
};

void playeresp::paint_traverse()
{
	static auto alpha = 1.0f;

	c_dormant_esp::get().start();

	if (g_cfg.player.arrows && g_ctx.local()->is_alive())
	{
		static auto switch_alpha = false;

		if (alpha <= 0.0f || alpha >= 1.0f)
			switch_alpha = !switch_alpha;

		alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
		alpha = math::clamp(alpha, 0.0f, 1.0f);
	}

	playeresp::speclist();
	playeresp::keylist();

	static auto FindHudElement = (DWORD(__thiscall*)(void*, const char*))util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	static auto hud_ptr = *(DWORD**)(util::FindSignature(crypt_str("client.dll"), crypt_str("81 25 ? ? ? ? ? ? ? ? 8B 01")) + 0x2);

	auto radar_base = FindHudElement(hud_ptr, "CCSGO_HudRadar");
	auto hud_radar = (CCSGO_HudRadar*)(radar_base - 0x14);

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(false, false))
			continue;

		type = ENEMY;

		if (e == g_ctx.local())
			type = LOCAL;
		else if (e->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
			type = TEAM;

		if (type == LOCAL && !m_input()->m_fCameraInThirdPerson)
			continue;

		auto valid_dormant = false;
		auto backup_flags = e->m_fFlags();
		auto backup_origin = e->GetAbsOrigin();

		if (e->IsDormant())
			valid_dormant = c_dormant_esp::get().adjust_sound(e);
		else
		{
			health[i] = e->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, e->GetAbsOrigin(), e->m_fFlags());
		}

		if (radar_base && hud_radar && e->IsDormant() && e->m_iTeamNum() != g_ctx.local()->m_iTeamNum() && e->m_bSpotted())
			health[i] = hud_radar->radar_info[i].health;

		if (!health[i])
		{
			if (e->IsDormant())
			{
				e->m_fFlags() = backup_flags;
				e->set_abs_origin(backup_origin);
			}

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime;
		auto slow = 0.25f * m_globals()->m_frametime;

		if (e->IsDormant())
		{
			auto origin = e->GetAbsOrigin();

			if (origin.IsZero())
				esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && esp_alpha_fade[i] > 0.0f)
				esp_alpha_fade[i] -= slow;
			else if (valid_dormant && esp_alpha_fade[i] < 1.0f)
				esp_alpha_fade[i] += fast;
		}
		else if (esp_alpha_fade[i] < 1.0f)
			esp_alpha_fade[i] += fast;

		esp_alpha_fade[i] = math::clamp(esp_alpha_fade[i], 0.0f, 1.0f);

		if (g_cfg.player.type[type].skeleton)
		{
			auto color = g_cfg.player.type[type].skeleton_color;
			color.SetAlpha(min(255.0f * esp_alpha_fade[i], color.a()));

			draw_skeleton(e, color, e->m_CachedBoneData().Base());
		}

		Box box;

		if (util::get_bbox(e, box, true))
		{
			draw_box(e, box);
			draw_name(e, box);

			auto& hpbox = hp_info[i];

			if (hpbox.hp == -1)
				hpbox.hp = math::clamp(health[i], 0, 100);
			else
			{
				auto hp = math::clamp(health[i], 0, 100);

				if (hp != hpbox.hp)
				{
					if (hpbox.hp > hp)
					{
						if (hpbox.hp_difference_time) //-V550
							hpbox.hp_difference += hpbox.hp - hp;
						else
							hpbox.hp_difference = hpbox.hp - hp;

						hpbox.hp_difference_time = m_globals()->m_curtime;
					}
					else
					{
						hpbox.hp_difference = 0;
						hpbox.hp_difference_time = 0.0f;
					}

					hpbox.hp = hp;
				}

				if (m_globals()->m_curtime - hpbox.hp_difference_time > 0.2f && hpbox.hp_difference)
				{
					auto difference_factor = 4.0f * m_globals()->m_frametime * hpbox.hp_difference;

					hpbox.hp_difference -= difference_factor;
					hpbox.hp_difference = math::clamp(hpbox.hp_difference, 0, 100);

					if (!hpbox.hp_difference)
						hpbox.hp_difference_time = 0.0f;
				}
			}

			draw_health(e, box, hpbox);
			draw_weapon(e, box, draw_ammobar(e, box));
			draw_flags(e, box);
		}

		if (type == ENEMY || type == TEAM)
		{

			if (type == ENEMY)
			{
				if (g_cfg.player.arrows &&

					g_ctx.local()->is_alive())
				{
					auto color = g_cfg.player.arrows_color;
					color.SetAlpha((int)(min(255.0f * esp_alpha_fade[i] * alpha, color.a())));

					if (e->IsDormant())
						color = Color(130, 130, 130, (int)(255.0f *

							esp_alpha_fade[i]));

					misc::get().PovArrows(e, color);
				}

				if (!e->IsDormant())
					draw_multi_points(e);
			}
		}

		if (e->IsDormant())
		{
			e->m_fFlags() = backup_flags;
			e->set_abs_origin(backup_origin);
		}
	}
}

int get_bind_state(key_bind t, int id)
{
	switch (t.mode) {
	case HOLD: if (m_inputsys()->IsButtonDown(t.key)) return 1; break;
	case TOGGLE: if (key_binds::get().get_key_bind_state(id)) return 2; break;
	}
	return 0;
}

void playeresp::anotaone() {
	if (!g_cfg.misc.centerinds)
		return;

	static int x = 0;
	static int y = 0;
	std::vector < std::pair < std::string, bool>> vec;

	if (!x || !y)
		m_engine()->GetScreenSize(x, y);

	Color nonactive = Color(g_cfg.misc.centerinds_inactive);
	Color activecol = Color(g_cfg.misc.centerinds_active);

	bool is_antiaim = g_cfg.antiaim.enable;
	bool is_doubletap = misc::get().double_tap_enabled;
	bool is_safepoint = get_bind_state(g_cfg.ragebot.safe_point_key, 3) > 0;
	bool is_hideshots = misc::get().hide_shots_enabled;
	bool is_inverted = get_bind_state(g_cfg.antiaim.flip_desync, 16) > 0;

	vec.push_back(std::make_pair("GLOBAL", is_antiaim));
	vec.push_back(std::make_pair("INVERTED", is_inverted));
	vec.push_back(std::make_pair("DOUBLETAP", is_doubletap));
	vec.push_back(std::make_pair("SAFEPOINT", is_safepoint));
	vec.push_back(std::make_pair("ONSHOT", is_hideshots));

	int cyc = 2;

	for (auto i : vec) {
		render::get().text(fonts[INDICATORS], x / 2, y / 2 + cyc * 18 + 6, i.second ? activecol : nonactive, HFONT_CENTERED_X, i.first.c_str());
		//render::get().text(fonts[NAME], x / 2 + 1, y / 2 + cyc * 18 + 1, Color(12, 12, 12), HFONT_CENTERED_X, i.first.c_str());
		++cyc;
	}
}

void playeresp::centerinds() {
	if (!g_cfg.misc.InfoBar)
		return;

	static int x = 0;
	static int y = 0;

	if (!x || !y)
		m_engine()->GetScreenSize(x, y);

	Vector2D scr = Vector2D(x / 2, y);

	auto& auf = g_ctx.local()->m_vecAbsVelocity();
	int speed = auf.Length();

	auto nci = m_engine()->GetNetChannelInfo();
	auto ping = 1.f / nci->GetLatency(FLOW_INCOMING);

	auto var = 1.f / m_globals()->m_absoluteframetime - 1.f / m_globals()->m_frametime;

	auto fps = 1.f / m_globals()->m_absoluteframetime;

	render::get().text(fonts[NAME], scr.x - 120 - 50, y - 20, Color(200, 255, 82, 255), NULL, "%d", (int)ping);
	render::get().text(fonts[ESP1], scr.x - 120 - 50 + 22, y - 18, Color(255, 255, 255), NULL, "ping");

	render::get().text(fonts[NAME], scr.x - 30 - 50, y - 20, Color(200, 255, 82, 255), NULL, "%d", (int)fps);
	render::get().text(fonts[ESP1], scr.x - 30 - 50 + 26, y - 18, Color(255, 255, 255), NULL, "fps");

	render::get().text(fonts[NAME], scr.x + 30, y - 20, Color(200, 255, 82, 255), NULL, "%d", (int)var);
	render::get().text(fonts[ESP1], scr.x + 30 + 22, y - 18, Color(255, 255, 255), NULL, "var");

	render::get().text(fonts[NAME], scr.x + 120, y - 20, Color(200, 255, 82, 255), NULL, "%d", (int)speed);
	render::get().text(fonts[ESP1], scr.x + 120 + 22, y - 18, Color(255, 255, 255), NULL, "speed");

	render::get().gradient(scr.x - 120 - 50 - 20 - 180, y - 28, 370, 28, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
	render::get().gradient(scr.x, y - 28, 420, 28, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
}

void playeresp::draw_skeleton(player_t* e, Color color, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	auto model = e->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	auto upper_direction = get_bone_position(7) - get_bone_position(6);
	auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (auto i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->pBone(i);

		if (!bone)
			continue;

		if (bone->parent == -1)
			continue;

		if (!(bone->flags & BONE_USED_BY_HITBOX))
			continue;

		auto child = get_bone_position(i);
		auto parent = get_bone_position(bone->parent);

		auto delta_child = child - breast_bone;
		auto delta_parent = parent - breast_bone;

		if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			parent = breast_bone;

		if (i == 5)
			child = breast_bone;

		if (fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f || i == 6)
			continue;

		auto schild = ZERO;
		auto sparent = ZERO;

		if (math::world_to_screen(child, schild) && math::world_to_screen(parent, sparent))
			render::get().line(schild.x, schild.y, sparent.x, sparent.y, color);
	}
}

void playeresp::draw_box(player_t* m_entity, const Box& box)
{
	if (!g_cfg.player.type[type].box)
		return;

	auto alpha = 255.0f * esp_alpha_fade[m_entity->EntIndex()];
	int outline_alpha = alpha * 2.0f;

	Color outline_color{ 0, 0, 0, outline_alpha };
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.type[type].box_color;

	color.SetAlpha(alpha);
	render::get().rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, outline_color);
	render::get().rect(box.x, box.y, box.w, box.h, color);
	render::get().rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, outline_color);
}

struct keybind {
	std::string name = "";
	int prev_act = 0;
	int mode = 0;
	float last_time_update = 0.f;
	float alpha = 0.f;
	key_bind keyshit;
};
struct sperma {
	key_bind& keybind;
	int id;
	const char* name;
};
void update_keybind(key_bind& keybd, int id) {
	if (id >= 4 && id < 12 && g_ctx.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4 + g_ctx.globals.current_weapon) && !g_ctx.globals.weapon->is_non_aim() ||
		id == 2 && misc::get().double_tap_enabled || id == 12 && misc::get().hide_shots_enabled || get_bind_state(keybd, id) > 0) {
		keybd.last_time_update = m_globals()->m_realtime;
	}
}

void playeresp::keylist() {

	if (!g_cfg.misc.keybinds)
		return;

	std::vector<sperma> keybinds = {
		{ g_cfg.antiaim.flip_desync, 16, "invert" },
		{g_cfg.misc.thirdperson_toggle, 17, "third person"},
		{g_cfg.antiaim.hide_shots_key, 12, "hide shots"},
		{g_cfg.misc.fakeduck_key, 20, "fake duck "},
		{g_cfg.misc.slowwalk_key, 21, "slow walk "},
		{g_cfg.ragebot.body_aim_key, 22, "body aim"},
		{g_cfg.ragebot.double_tap_key, 2, "double tap"},
		{g_cfg.ragebot.safe_point_key, 3, "safe points"},
		{g_cfg.misc.automatic_peek, 18, "auto peek"},
		{g_cfg.antiaim.manual_back, 13, "back manual"},
		{g_cfg.antiaim.manual_left, 14, "left manual"},
		{g_cfg.antiaim.manual_right, 15, "right manual"},
		{g_cfg.misc.edge_jump, 19, "edge jump"}
	};

	for (auto& keybind : keybinds) {
		update_keybind(keybind.keybind, keybind.id);
	}

	const auto time = m_globals()->m_realtime;
	constexpr auto anim_speed = 0.2f;

	auto pos = Vector2D(600, 200);
	auto size = Vector2D(140.f, 20.f);

	constexpr auto padding = 4.f;
	constexpr auto spacing = 4.f;
	constexpr auto line_size = 2.f;

	auto back_color = Color(0.f, 0.f, 0.f, 0.95f);
	auto text_color = Color(1.f, 1.f, 1.f, 0.95f);
	auto line_color = Color(0.6f, 0.8f, 1.f);

	auto last_expiration = 0.f;

	std::vector<sperma> active;
	for (auto &s2 : keybinds) {
		auto cal2 = &s2.keybind;

		const auto expiration = 1.f - (time - cal2->last_time_update) / anim_speed;

		last_expiration = max(last_expiration, expiration);

		if (expiration <= 0.f)
			continue;

		cal2->alpha = expiration * 255.f;

		active.push_back(s2);
	}

	if (active.empty())
		return;

	line_color.SetAlpha(last_expiration * 255.f);
	back_color.SetAlpha(last_expiration * 255.f);
	text_color.SetAlpha(last_expiration * 255.f);

	render::get().rect_filled(pos.x, pos.y, size.x, line_size, line_color);
	render::get().rect_filled(pos.x, pos.y + line_size, size.x, size.y - line_size, back_color);

	auto posay = pos + (size / 2.f);

	render::get().text(fonts[NAME], posay.x, posay.y, text_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, crypt_str("keybindings"));

	auto draw_pos = Vector2D(padding, size.y + spacing);
	auto draw_size = Vector2D(size.x - (padding * 2.f), 0.f);

	for (auto& data : active) {
		auto mode = data.keybind.mode == 0 ? "hold" : "toggle";
		const auto text_size = Vector2D(render::get().text_width(fonts[NAME], mode), render::get().text_heigth(fonts[NAME], mode));

		auto govon_pos = pos + draw_pos + draw_size;

		text_color.SetAlpha((int)data.keybind.alpha);

		render::get().text(fonts[NAME], pos.x + draw_pos.x, pos.y + draw_pos.y, text_color, HFONT_CENTERED_NONE, data.name);
		render::get().text(fonts[NAME], govon_pos.x - text_size.x, govon_pos.y, text_color, HFONT_CENTERED_NONE, mode);

		draw_pos.y += text_size.y + spacing;
	}
}

bool mouse_pointer(Vector2D position, Vector2D size)
{
	return hooks::mouse_pos.x > position.x && hooks::mouse_pos.y > position.y && hooks::mouse_pos.x < position.x + size.x && hooks::mouse_pos.y < position.y + size.y;
}

int IsCheater1() {

	const auto choked_ticks = m_clientstate()->iChokedCommands;

	return choked_ticks;
}

void playeresp::speclist()
{
	if (!g_cfg.misc.spectators_list)
		return;

	static Vector2D tl(150, 150);

	static bool _dragging = false;
	bool _click = false;

	static int drag_x = 50;
	static int drag_y = 50;

	if (hooks::menu_open && GetAsyncKeyState(VK_LBUTTON))
		_click = true;

	Vector2D _mouse_pos = hooks::mouse_pos;

	if (hooks::menu_open)
	{
		if (_dragging && !_click)
			_dragging = false;

		if (_dragging && _click)
		{
			tl.x = _mouse_pos.x - drag_x;
			tl.y = _mouse_pos.y - drag_y;
		}



		if (mouse_pointer(Vector2D(tl.x, tl.y), Vector2D(130, 20)))
		{
			_dragging = true;
			drag_x = _mouse_pos.x - tl.x;
			drag_y = _mouse_pos.y - tl.y;
		}
	}

	const auto pos = Vector2D(tl.x, tl.y);
	const auto padding = 4;

	const auto col_background = Color(0, 0, 0, 0);
	const auto col_accent = Color(g_cfg.esp.spectatorss);
	const auto col_text = Color(255, 255, 255);
	const auto bg_size = Vector2D(100, 20);

	const std::string name = "spectators";
	int valid = 0;

	if (g_ctx.local() && g_ctx.local()->is_alive() && m_engine()->IsConnected() && m_engine()->IsInGame())
	{
		for (int i = 1; i < 64; i++)
		{
			auto p_local = static_cast<player_t*>(m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()));
			auto p_entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

			if (!p_local && !p_local->is_alive())
				return;

			player_info_t e_info;

			if (p_entity && p_entity != p_local) {

				m_engine()->GetPlayerInfo(i, &e_info);

				if (!p_entity->is_alive() && !p_entity->IsDormant())
				{
					auto target = p_entity->m_hObserverTarget();
					if (target.IsValid())
					{
						auto p_target = m_entitylist()->GetClientEntityFromHandle(target);
						if (p_target == p_local)
						{
							valid++;
							render::get().text(fonts[NAME], pos.x + bg_size.x / 2, pos.y + padding + valid * 20, col_text, HFONT_CENTERED_X, e_info.szName);
						}
					}
				}
			}
		}
	}


	render::get().rect_filled(pos.x, pos.y, bg_size.x, bg_size.y, col_background);
	render::get().rect_filled(pos.x, pos.y, bg_size.x, 2, col_accent);
	render::get().text(fonts[NAME], pos.x + (bg_size.x / 2), pos.y + padding, col_text, HFONT_CENTERED_X, name.c_str());
}

int limit(int to, int max) {
	int ret = 0;
	if (to > max)
		ret = max;
	else
		ret = to;
	return ret;
}

void playeresp::evolveinds()
{
	if (!g_cfg.misc.evoinds) // замени на свое
		return;

	static Vector2D tl(110, 110);

	static bool _dragging = false;
	bool _click = false;

	static int drag_x = 110;
	static int drag_y = 110;

	Vector2D _mouse_pos = hooks::mouse_pos;

	if (hooks::menu_open && GetAsyncKeyState(VK_LBUTTON))
		_click = true;

	if (hooks::menu_open)
	{
		if (_dragging && !_click)
			_dragging = false;

		if (_dragging && _click)
		{
			tl.x = _mouse_pos.x - drag_x;
			tl.y = _mouse_pos.y - drag_y;
		}

		if (mouse_pointer(Vector2D(tl.x, tl.y), Vector2D(150, 20)))
		{
			_dragging = true;
			drag_x = _mouse_pos.x - tl.x;
			drag_y = _mouse_pos.y - tl.y;
		}
	}

	int min_value;

	if (g_ctx.local()->m_vecVelocity().Length2D() > 112)
	{
		min_value = 60 / (g_ctx.local()->m_vecVelocity().Length2D() / 122);
	}
	else
	{
		min_value = 60;
	}

	// spectators list

	int desync = min_value;

	//vars

	Vector2D pos = Vector2D(tl.x, tl.y);
	static int height = 65; // height of inds
	static int width = 180; // width of inds

	render::get().rect(pos.x, pos.y, width, height * 2, Color(255, 255, 255, 255));
	render::get().rect_filled(pos.x + 1, pos.y + 1, width - 2, height * 2 - 2, Color(26, 26, 26, 255));

	// fps counter vars
	float m_Framerate = 0; m_Framerate = (0.9 * m_Framerate + (1.0 - 0.9) * m_globals()->m_absoluteframetime) * 10; // gets fps
	int formated = (int)(1.0f / m_Framerate); // formatting fps
	float tickRate = 1.0f / m_globals()->m_intervalpertick; // server's tickrate (non-static)
	//choke vars
	auto chocked_commands = m_clientstate()->iChokedCommands; // chocked cmds
	//auto chocked_commands = *(int*)(uintptr_t(interfaces::client_state) + 0x4D28); // chocked cmds
	//FRAMES SECTION
	//renderer->draw_rect(Vec4(50 - 10 - 1, pos + 10 - 1, 20 + 1, height * 2 - 40 + 1), 1.f, D3DCOLOR_RGBA(42, 42, 42, 255)); //outlined fill
	render::get().rect_filled(50 - 10 + pos.x - 10, pos.y + 10, 20, height * 2 - 40, Color(255, 255, 255, 255));
	render::get().rect_filled(50 - 10 + pos.x - 10, pos.y + 10, 20, (height * 2 - 40) * abs((limit(formated, (int)tickRate) / tickRate) - 1), Color(26, 26, 26, 255));
	render::get().text(fonts[ESP], pos.x + 40, pos.y + height * 1.6, Color(255, 255, 255, 255), HFONT_CENTERED_X, "Frames");

	//CHOKE SECTION
	//renderer->draw_rect(Vec4(100 - 10 - 1, pos + 10 - 1, 20 + 1, height * 2 - 40 + 1), 1.f, D3DCOLOR_RGBA(42, 42, 42, 255)); //outlined fill
	render::get().rect_filled(100 - 10 + pos.x - 10, pos.y + 10, 20, height * 2 - 40, Color(255, 255, 255, 255));
	render::get().rect_filled(100 - 10 + pos.x - 10, pos.y + 10, 20, (height * 2 - 40) * abs((chocked_commands / 16.f) - 1), Color(26, 26, 26, 255));
	render::get().text(fonts[ESP], 20 + 40 + 40 + pos.x - 10, pos.y + height * 1.6, Color(255, 255, 255, 255), HFONT_CENTERED_X, "Choke");
	//FAKE SECTION
	//renderer->draw_rect(Vec4(150 - 10 - 1, pos + 10 - 1, 20 + 1, height * 2 - 40 + 1), 1.f, D3DCOLOR_RGBA(42, 42, 42, 255)); //outlined fill
	render::get().rect_filled(150 - 10 + pos.x - 10, pos.y + 10, 20, height * 2 - 40, Color(255, 255, 255, 255));
	render::get().rect_filled(150 - 10 + pos.x - 10, pos.y + 10, 20, (height * 2 - 40) * abs(((abs(g_ctx.local()->get_max_desync_delta()) - 29) / 29) - 1), Color(26, 26, 26, 255));
	render::get().text(fonts[ESP], 150 + pos.x - 10, pos.y + height * 1.6, Color(255, 255, 255, 255), HFONT_CENTERED_X, "Desync");
}

void playeresp::draw_health(player_t* m_entity, const Box& box, const HPInfo& hpbox)
{
	if (!g_cfg.player.type[type].health)
		return;

	int player_health = m_entity->m_iHealth() > 100 ? 100 : m_entity->m_iHealth();
	if (!player_health)
		return;

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);

	auto text_color = m_entity->IsDormant() ? Color(130, 130, 130, alpha) : Color(255, 255, 255, alpha);
	auto back_color = Color(0, 0, 0, (int)(alpha * 1.f));
	auto color = m_entity->IsDormant() ? Color(130, 130, 130) : Color(150, (int)min(255.0f, hpbox.hp * 225.0f / 100.0f), 0);
	auto hp_effect_color = Color(215, 20, 20, alpha);

	if (g_cfg.player.type[type].custom_health_color)
		color = m_entity->IsDormant() ? Color(0, 0, 0, 255) : g_cfg.player.type[type].health_color;

	int outline_alpha = alpha * 0.6f;

	Color outline_color{ 0, 0, 0, outline_alpha };

	color.SetAlpha(alpha);
	render::get().rect(box.x - 6, box.y, 4, box.h, outline_color);

	Box n_box = {
		box.x - 5,
		box.y + 1,
		2,
		box.h - 2
	};

	auto bar_height = (int)((float)hpbox.hp * (float)n_box.h / 100.0f);
	auto offset = n_box.h - bar_height;

	render::get().rect_filled(n_box.x, n_box.y, 2, n_box.h, back_color);
	render::get().rect_filled(n_box.x, n_box.y + offset, 2, bar_height, color);

	auto height = n_box.h - hpbox.hp * n_box.h / 100;

	if (player_health < 100) {
		render::get().text(fonts[ESP],
			n_box.x + 1, n_box.y + offset,
			text_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, std::to_string(player_health).c_str()
		);
	}
}

bool NonAim(player_t* x) {

	if (x->m_hActiveWeapon()->is_non_aim())
		return true;

	return false;
}

bool playeresp::draw_ammobar(player_t* m_entity, const Box& box)
{
	if (!m_entity->is_alive())
		return false;

	if (!g_cfg.player.type[type].ammo)
		return false;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return false;

	int ammo = weapon->m_iClip1();

	auto alpha = (int)(255.0f * esp_alpha_fade[m_entity->EntIndex()]);
	int outline_alpha = alpha * 0.4f;
	int inner_back_alpha = alpha * 0.2f;

	Color outline_color{ 0, 0, 0, outline_alpha };
	Color inner_back_color{ 0, 0, 0, inner_back_alpha };
	auto text_color = Color(255, 255, 255, alpha);
	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.type[type].ammobar_color;

	color.SetAlpha(alpha);

	Box n_box = {
	box.x + 1,
	box.y + box.h + 3,
	box.w - 4,
	2
	};

	float bar_width = ammo * box.w / weapon->get_csweapon_info()->iMaxClip1;

	AnimationLayer animlayer = m_entity->get_animlayers()[1];

	if (animlayer.m_pOwner) {
		auto activity = m_entity->sequence_activity(animlayer.m_nSequence);

		bool reloading = (activity == 967 && animlayer.m_flWeight != 0.0f);

		if (reloading && animlayer.m_flCycle < 0.99)
			bar_width = (animlayer.m_flCycle * box.w) / 1.f;
	}


	render::get().rect_filled(n_box.x - 0.2 - 1, n_box.y - 0.2, box.w + 2, 4, Color(0, 0, 0, alpha));
	render::get().rect_filled(n_box.x - 1, n_box.y, bar_width, 2, color);

	return true;
}

void playeresp::draw_name(player_t* m_entity, const Box& box)
{
	if (!g_cfg.player.type[type].name)
		return;

	static auto sanitize = [](char* name) -> std::string
	{
		name[127] = '\0';

		std::string tmp(name);

		if (tmp.length() > 20)
		{
			tmp.erase(20, tmp.length() - 20);
			tmp.append("...");
		}

		return tmp;
	};

	player_info_t player_info;

	if (m_engine()->GetPlayerInfo(m_entity->EntIndex(), &player_info))
	{
		auto name = sanitize(player_info.szName);

		auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.type[type].name_color;
		color.SetAlpha(min(255.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

		render::get().text(fonts[NAME], box.x + box.w / 2, box.y - 13, color, HFONT_CENTERED_X, name.c_str());
	}
}

void playeresp::draw_weapon(player_t* m_entity, const Box& box, bool space)
{
	if (!g_cfg.player.type[type].weapon[PLAYER_DIST] && !g_cfg.player.type[type].weapon[WEAPON_ICON1] && !g_cfg.player.type[type].weapon[WEAPON_TEXT1])
		return;

	auto weapon = m_entity->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto pos = box.y + box.h + 3;

	if (space)
		pos += 3;

	auto color = m_entity->IsDormant() ? Color(130, 130, 130, 130) : g_cfg.player.type[type].weapon_color;
	color.SetAlpha(min(200.0f * esp_alpha_fade[m_entity->EntIndex()], color.a()));

	if (g_cfg.player.type[type].weapon[PLAYER_DIST])
	{

		auto distance = g_ctx.local()->GetAbsOrigin().DistTo(m_entity->GetAbsOrigin()) / 12.0f;

		render::get().text(fonts[ESP], box.x + box.w / 2, pos, color, HFONT_CENTERED_X, "%iFT", (int)distance);
		pos += 9;
	}

	if (g_cfg.player.type[type].weapon[WEAPON_ICON])
	{
		if (weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE)
			render::get().text(fonts[KNIFES], box.x + box.w / 2, pos, color, HFONT_CENTERED_X, "z");
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_KNIFE_T)
			render::get().text(fonts[KNIFES], box.x + box.w / 2, pos, color, HFONT_CENTERED_X, "W");
		else
			render::get().text(fonts[WEAPONICONS], box.x + box.w / 2, pos, color, HFONT_CENTERED_X, weapon->get_icon());
		pos += 15;
	}

	if (g_cfg.player.type[type].weapon[WEAPON_TEXT])
	{
		render::get().text(fonts[ESP], box.x + box.w / 2, pos, color, HFONT_CENTERED_X, weapon->get_name().c_str());
		pos += 9;
	}

}

void playeresp::draw_flags(player_t* e, const Box& box)
{
	auto weapon = e->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto _x = box.x + box.w + 3, _y = box.y - 3;

	if (g_cfg.player.type[type].flags[FLAGS_MONEY])
	{
		auto color = e->IsDormant() ? Color(115, 186, 0, 130) : Color(115, 186, 0);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, "$%i", e->m_iAccount());
		_y += 8;
	}

	if (g_cfg.player.type[type].flags[FLAGS_ARMOR])
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		auto kevlar = e->m_ArmorValue() > 0;
		auto helmet = e->m_bHasHelmet();

		std::string text;

		if (helmet && kevlar)
			text = "HK";
		else if (kevlar)
			text = "K";

		if (kevlar)
		{
			render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, text.c_str());
			_y += 8;
		}
	}

	if (g_cfg.player.type[type].flags[FLAGS_KIT] && e->m_bHasDefuser())
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(240, 240, 240);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, "KIT");
		_y += 8;
	}

	if (g_cfg.player.type[type].flags[FLAGS_SCOPED])
	{
		auto scoped = e->m_bIsScoped();

		if (e == g_ctx.local())
			scoped = g_ctx.globals.scoped;

		if (scoped)
		{
			auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(30, 120, 235);
			color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

			render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, "ZOOM");
			_y += 8;
		}
	}

	if (g_cfg.player.type[type].flags[FLAGS_FAKEDUCKING])
	{
		auto animstate = e->get_animation_state();

		if (animstate)
		{
			auto fakeducking = [&]() -> bool
			{
				static auto stored_tick = 0;
				static int crouched_ticks[65];

				if (animstate->m_fDuckAmount)
				{
					if (animstate->m_fDuckAmount < 0.9f && animstate->m_fDuckAmount > 0.5f)
					{
						if (stored_tick != m_globals()->m_tickcount)
						{
							crouched_ticks[e->EntIndex()]++;
							stored_tick = m_globals()->m_tickcount;
						}

						return crouched_ticks[e->EntIndex()] > 16;
					}
					else
						crouched_ticks[e->EntIndex()] = 0;
				}

				return false;
			};

			if (fakeducking() && e->m_fFlags() & FL_ONGROUND && !animstate->m_bInHitGroundAnimation)
			{
				auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(215, 20, 20);
				color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

				render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, "FD");
				_y += 8;
			}
		}
	}

	if (g_cfg.player.type[type].flags[FLAGS_C4] && e->EntIndex() == g_ctx.globals.bomb_carrier)
	{
		auto color = e->IsDormant() ? Color(130, 130, 130, 130) : Color(163, 49, 93);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);

		render::get().text(fonts[ESP], _x, _y, color, HFONT_CENTERED_NONE, "B");
		_y += 8;
	}

	if (g_cfg.player.type[type].flags[light_font] && e->m_hActiveWeapon()->m_iItemDefinitionIndex() == WEAPON_TASER)//g_ctx.globals.fired_shot
	{
		auto color = e->IsDormant() ? Color(255, 255, 0, 255) : Color(255, 255, 0, 255);
		color.SetAlpha(255.0f * esp_alpha_fade[e->EntIndex()]);
		render::get().text(fonts[light_font], _x, _y, color, HFONT_CENTERED_NONE, "r");
		//graphics->unsafe_text(bbox.left + (bbox.right / 2) - width * 1.04, bbox.top - 16, col_t(255, 255, 0, 255), graphics->fonts.light_font, false, "r");
		_y += 8;

	}

	//if (m_cfg.esp.zeus)
	//	if ((e->get_active_weapon())->get_item_definition_index() == WEAPON_GLOCK18)
			//graphics->unsafe_text(bbox.left + (bbox.right / 2) - width * 1.04, bbox.top - 16, col_t(255, 255, 0, 255), graphics->fonts.light_font, false, "r");
}

void playeresp::draw_multi_points(player_t* e)
{
	if (!g_cfg.ragebot.enable)
		return;

	if (!g_cfg.player.show_multi_points)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	if (g_ctx.local()->get_move_type() == MOVETYPE_NOCLIP)
		return;

	if (g_ctx.globals.current_weapon == -1)
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	auto records = &player_records[e->EntIndex()];

	if (records->empty())
		return;

	auto record = &records->front();

	if (!record->valid(false))
		return;

	std::vector <scan_point> points;
	auto hitboxes = aim::get().get_hitboxes(record);

	for (auto& hitbox : hitboxes)
	{
		auto current_points = aim::get().get_points(record, hitbox, false);

		for (auto& point : current_points)
			points.emplace_back(point);
	}

	if (points.empty())
		return;

	for (auto& point : points)
	{
		Vector screen;

		if (!math::world_to_screen(point.point, screen))
			continue;

		render::get().rect_filled(screen.x - 1, screen.y - 1, 3, 3, g_cfg.player.show_multi_points_color);
		render::get().rect(screen.x - 2, screen.y - 2, 5, 5, Color::Black);
	}
}