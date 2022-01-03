// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\..\ImGui\imgui.h"
#include "..\lagcompensation\local_animations.h"
#include "../../utils/draw_manager.h"

bool can_penetrate(weapon_t* weapon)
{
	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	CTraceFilter filter;
	filter.pSkip = g_ctx.local();

	trace_t trace;
	util::trace_line(g_ctx.globals.eye_pos, g_ctx.globals.eye_pos + direction * weapon_info->flRange, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &trace);

	if (trace.fraction == 1.0f) //-V550
		return false;

	auto eye_pos = g_ctx.globals.eye_pos;
	auto hits = 1;
	auto damage = (float)weapon_info->iDamage;
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

	return autowall::get().handle_bullet_penetration(weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat());
}

void otheresp::draw_arc(int x, int y, float r1, float r2, int s, int d, Color m_clr)
{
	for (int i = s; i < s + d; i++)
	{
		auto rad = i * M_PI / 180;
		render::get().line(x + std::cos(rad) * r1, y + std::sin(rad) * r1, x + std::cos(rad) * r2, y + std::sin(rad) * r2, Color(m_clr.r(), m_clr.g(), m_clr.b(), m_clr.a()));
	}
}

void otheresp::penetration_reticle()
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.penetration_reticle)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto color = Color::Red;

	if (!weapon->is_non_aim() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && can_penetrate(weapon))
		color = Color::Green;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	render::get().rect_filled(width / 2, height / 2 - 1, 1, 3, color);
	render::get().rect_filled(width / 2 - 1, height / 2, 3, 1, color);
}

void otheresp::indicators()
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static int height, width;
	m_engine()->GetScreenSize(width, height);

	g_ctx.globals.indicator_pos = height / 2;
	if (g_cfg.esp.indicators[INDICATOR_HS] && g_cfg.antiaim.hide_shots && g_cfg.antiaim.hide_shots_key.key > KEY_NONE && g_cfg.antiaim.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key) {
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "HS") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "HS") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "HS") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "HS") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "HS") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "HS");
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, Color(240, 240, 240, 255), 0, "HS");
		g_ctx.globals.indicator_pos += 45;
	}
	if (g_cfg.esp.indicators[INDICATOR_DT] && g_cfg.ragebot.double_tap && g_cfg.ragebot.double_tap_key.key > KEY_NONE && g_cfg.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key) {
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DT") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DT") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "DT") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DT") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DT") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "DT");
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, !g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().double_tap_enabled && !weapon->is_grenade() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && weapon->can_fire(false) ? Color(240, 240, 240, 255) : Color(130, 20, 0, 255), 0, "DT");
		g_ctx.globals.indicator_pos += 45;
	}
	if (g_cfg.esp.indicators[INDICATOR_DAMAGE] && g_ctx.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4 + g_ctx.globals.current_weapon) && !weapon->is_non_aim()) {
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DMG: 00") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DMG: 00") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "DMG: 00") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DMG: 00") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DT") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "DMG: %d", g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].minimum_override_damage);
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, Color(130, 240, 0, 255), 0, "DMG: %d", g_cfg.ragebot.weapon[g_ctx.globals.current_weapon].minimum_override_damage);
		g_ctx.globals.indicator_pos += 45;
	}


	if (g_cfg.antiaim.desync) {
		auto colorfake = Color(130, 20 + (int)(((g_ctx.local()->get_max_desync_delta() - 29.f) / 29.f) * 200.0f), 0);
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "FAKE ") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "FAKE ") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "FAKE ") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "FAKE ") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DT") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "FAKE");
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, colorfake, 0, "FAKE");
		render::get().draw_arc(10 + render::get().text_width(fonts[INDICATORFONT2], "FAKE") + 12, g_ctx.globals.indicator_pos + 13, 7, NULL, 360, 4, Color(0, 0, 0, 200));
		render::get().draw_arc(10 + render::get().text_width(fonts[INDICATORFONT2], "FAKE") + 12, g_ctx.globals.indicator_pos + 13, 7, NULL, ((g_ctx.local()->get_max_desync_delta() - 29.f) / 29.f) * 360.f, 4, colorfake);
		g_ctx.globals.indicator_pos += 45;
	}
	if (g_cfg.antiaim.fakelag) {
		auto colorfl = Color(130, 20 + (int)((m_clientstate()->iChokedCommands / 15.f) * 200.0f), 0);
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "FL ") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "FL ") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "FL ") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "FL ") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DT") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "FL");
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, colorfl, 0, "FL");
		render::get().draw_arc(10 + render::get().text_width(fonts[INDICATORFONT2], "FL") + 12, g_ctx.globals.indicator_pos + 13, 7, NULL, 360, 4, Color(0, 0, 0, 200));
		render::get().draw_arc(10 + render::get().text_width(fonts[INDICATORFONT2], "FL") + 12, g_ctx.globals.indicator_pos + 13, 7, NULL, (m_clientstate()->iChokedCommands / 15.f) * 360.f, 4, colorfl);
		g_ctx.globals.indicator_pos += 45;
	}


	if (g_cfg.esp.indicators[INDICATOR_DUCK] && key_binds::get().get_key_bind_state(20)) {
		render::get().gradient(10, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DUCK") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DUCK") + 8, Color(0, 0, 0, 0), Color(0, 0, 0, 200), GRADIENT_HORIZONTAL);
		render::get().gradient(10 + render::get().text_width(fonts[INDICATORFONT2], "DUCK") / 2, g_ctx.globals.indicator_pos - 4, render::get().text_width(fonts[INDICATORFONT2], "DUCK") / 2, render::get().text_heigth(fonts[INDICATORFONT2], "DUCK") + 8, Color(0, 0, 0, 200), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);

		render::get().text(fonts[INDICATORFONT2], 11, g_ctx.globals.indicator_pos + 1, Color::Black, 0, "DUCK");
		render::get().text(fonts[INDICATORFONT2], 10, g_ctx.globals.indicator_pos, Color(240, 240, 240, 255), 0, "DUCK");
		g_ctx.globals.indicator_pos += 45;
	}
}


void render::draw_arc(int x, int y, int radius, int startangle, int percent, int thickness, Color color) {
	auto precision = (2 * M_PI) / 30;
	auto step = M_PI / 180;
	auto inner = radius - thickness;
	auto end_angle = (startangle + percent) * step;
	auto start_angle = (startangle * M_PI) / 180;

	for (; radius > inner; --radius) {
		for (auto angle = start_angle; angle < end_angle; angle += precision) {
			auto cx = round(x + radius * cos(angle));
			auto cy = round(y + radius * sin(angle));

			auto cx2 = round(x + radius * cos(angle + precision));
			auto cy2 = round(y + radius * sin(angle + precision));

			line(cx, cy, cx2, cy2, color);
		}
	}
}

void draw_circle(int x, int y, int radius, int thickness, Color color)
{
	auto inner = radius - thickness;

	for (; radius > inner; --radius)
	{
		render::get().circle(x, y, 30, radius, color);
	}
}

float adjust_angle(float angle)
{
	if (angle < 0)
	{
		angle = (90 + angle * (-1));
	}
	else if (angle > 0)
	{
		angle = (90 - angle);
	}

	return angle;
}

void otheresp::circlefake() {

	if (!g_ctx.local())
		return;

	auto cvar = m_cvar()->FindVar("cl_crosshairalpha");
	static auto old_alpha = cvar->GetFloat();

	if (cvar)
		if (g_cfg.esp.circlefake)
			cvar->SetValue(0.f);
		else {
			cvar->SetValue(old_alpha);
			return;
		}

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	auto x = width / 2, y = height / 2;

	if (!local_animations::get().local_data.animstate)
		return;

	float real_yaw = g_ctx.local()->get_animation_state()->m_flGoalFeetYaw;
	float fake_yaw = local_animations::get().local_data.animstate->m_flGoalFeetYaw;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);
	auto view_yaw = view_angles[1] - 180;

	auto real = adjust_angle(real_yaw - view_yaw);
	auto fake = adjust_angle(fake_yaw - view_yaw);

	auto arc_length = 50;
	auto circle_radius = 16;
	auto arc_thickness = 4;

	auto col = Color::Black;
	col.SetAlpha(100);

	draw_circle(x, y, 10, 5, col);

	if (math::normalize_yaw(real) > 90.f)
		render::get().draw_arc(x, y, 10, 90, 170, 4, Color(g_cfg.esp.circlefake_color));
	else
		render::get().draw_arc(x, y, 10, 270, 170, 4, Color(g_cfg.esp.circlefake_color));

	render::get().draw_arc(x, y, circle_radius, fake - (arc_length * 0.5), arc_length, arc_thickness, Color(g_cfg.esp.circlefake_color));
	//render::get().text(fonts[ESP], 100, 100, Color(255, 255, 255), 0, "%.0f", real);

}


void otheresp::fake() {
	if (!g_cfg.misc.fakeline)
		return;

	if (!g_ctx.local())
		return;

	static int height = 0, width = 0;
	if (!height || !width)
		m_engine()->GetScreenSize(width, height);

	if (!local_animations::get().local_data.animstate)
		return;

	auto body_yaw = max(-60, min(60, std::round(g_ctx.local()->m_flPoseParameter()[11] * 120 - 58 + 0.5), 1));

	float to_draw = key_binds::get().get_key_bind_state(16) ? body_yaw : body_yaw * -1;

	wchar_t auf[12];
	swprintf_s(auf, 11, L"%.0f°", to_draw);

	render::get().wtext(fonts[NAME], width / 2, height / 2 + 12 + 8, Color(255, 255, 255, 255), HFONT_CENTERED_X, auf);
	render::get().gradient(width / 2, height / 2 + 12 + 14 + 8, 28 + 3 + std::clamp(fabs(to_draw), 0.f, to_draw), 3, Color(g_cfg.misc.fakeline_color), Color(255, 255, 255, 0), GRADIENT_HORIZONTAL);
	render::get().gradient(width / 2 - 28 - std::clamp(fabs(to_draw), 0.f, to_draw), height / 2 + 12 + 14 + 8, 28 + std::clamp(fabs(to_draw), 0.f, to_draw), 3, Color(255, 255, 255, 0), Color(g_cfg.misc.fakeline_color), GRADIENT_HORIZONTAL);
}

std::string parseString(const std::string& szBefore, const std::string& szSource)
{
	if (!szBefore.empty() && !szSource.empty() && (szSource.find(szBefore) != std::string::npos))
	{
		std::string t = strstr(szSource.c_str(), szBefore.c_str()); //-V522
		t.erase(0, szBefore.length());
		size_t firstLoc = t.find('\"', 0);
		size_t secondLoc = t.find('\"', firstLoc + 1);
		t = t.substr(firstLoc + 1, secondLoc - 3);
		return t;
	}
	else
		return crypt_str("");
}

void otheresp::draw_velocity()
{
	if (!g_cfg.esp.velocity_graph)
		return;

	if (!g_ctx.local())
		return;

	if (!m_engine()->IsInGame() || !g_ctx.local()->is_alive())
		return;

	static std::vector<float> velData(120, 0);

	Vector vecVelocity = g_ctx.local()->m_vecVelocity();
	float currentVelocity = sqrt(vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y);

	velData.erase(velData.begin());
	velData.push_back(currentVelocity);

	int vel = g_ctx.local()->m_vecVelocity().Length2D();

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	render::get().text(fonts[VELOCITY], width / 2, height / 1.1, Color(0, 255, 100, 255), HFONT_CENTERED_X | HFONT_CENTERED_Y, "(%i)", vel);


	for (auto i = 0; i < velData.size() - 1; i++)
	{
		int cur = velData.at(i);
		int next = velData.at(i + 1);

		render::get().line(
			width / 2 + (velData.size() * 5 / 2) - (i - 1) * 5.f,
			height / 1.15 - (std::clamp(cur, 0, 450) * .2f),
			width / 2 + (velData.size() * 5 / 2) - i * 5.f,
			height / 1.15 - (std::clamp(next, 0, 450) * .2f), Color(255, 255, 255, 255)
		);
	}
}

void otheresp::draw_indicators()
{
	if (!g_ctx.local()->is_alive()) //-V807
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto h = height / 2 + 50;

	for (auto& indicator : m_indicators)
	{
		render::get().gradient(5, h - 15, 30, 30, Color(0, 0, 0, 0), Color(0, 0, 0, 150), GRADIENT_HORIZONTAL);
		render::get().gradient(35, h - 15, 30, 30, Color(0, 0, 0, 150), Color(0, 0, 0, 0), GRADIENT_HORIZONTAL);
		render::get().text(fonts[INDICATORFONT1], 10, h, indicator.m_color, HFONT_CENTERED_Y, indicator.m_text.c_str());
		h += 35;
	}

	m_indicators.clear();
}

void otheresp::hitmarker_paint()
{
	//for (auto i = 1; i < m_globals()->m_maxclients; i++) //-V807
	{
		if (!g_cfg.esp.hitmarker[0] && !g_cfg.esp.hitmarker[1])
		{
			hitmarker.hurt_time = FLT_MIN;
			hitmarker.point = ZERO;
			return;
		}

		if (!g_ctx.local()->is_alive())
		{
			hitmarker.hurt_time = FLT_MIN;
			hitmarker.point = ZERO;
			return;
		}


		if (hitmarker.hurt_time + 2.0f > m_globals()->m_curtime)
		{
			auto hitmark_col = Color(240, 240, 240, 240);
			if (g_cfg.esp.hitmarker[0])
			{
				static int width, height;
				m_engine()->GetScreenSize(width, height);

				auto alpha = (int)((hitmarker.hurt_time + 2.0f - m_globals()->m_curtime) * 127.5f);
				hitmark_col.SetAlpha(alpha);

				auto offset = 1.0f - (float)alpha / 255.0f * 1.0f;

				render::get().line(width / 2 + 4 + offset, height / 2 - 4 - offset, width / 2 + 8 + offset, height / 2 - 8 - offset, hitmark_col);
				render::get().line(width / 2 + 4 + offset, height / 2 + 4 + offset, width / 2 + 8 + offset, height / 2 + 8 + offset, hitmark_col);
				render::get().line(width / 2 - 4 - offset, height / 2 + 4 + offset, width / 2 - 8 - offset, height / 2 + 8 + offset, hitmark_col);
				render::get().line(width / 2 - 4 - offset, height / 2 - 4 - offset, width / 2 - 8 - offset, height / 2 - 8 - offset, hitmark_col);
			}

			if (g_cfg.esp.hitmarker[1])
			{
				Vector world;

				if (math::world_to_screen(hitmarker.point, world))
				{
					auto alpha = (int)((hitmarker.hurt_time + 2.0f - m_globals()->m_curtime) * 127.5f);
					hitmark_col.SetAlpha(alpha);

					auto offset = 1.0f - (float)alpha / 255.0f * 1.0f;

					render::get().line(world.x + 4 + offset, world.y - 4 - offset, world.x + 8 + offset, world.y - 8 - offset, hitmark_col);
					render::get().line(world.x + 4 + offset, world.y + 4 + offset, world.x + 8 + offset, world.y + 8 + offset, hitmark_col);
					render::get().line(world.x - 4 - offset, world.y + 4 + offset, world.x - 8 - offset, world.y + 8 + offset, hitmark_col);
					render::get().line(world.x - 4 - offset, world.y - 4 - offset, world.x - 8 - offset, world.y - 8 - offset, hitmark_col);
				}
			}
		}
	}
}

void otheresp::damage_marker_paint()
{

	for (auto i = 1; i <= m_globals()->m_maxclients; i++)
	{
		if (damage_marker[i].hurt_time + 2.0f > m_globals()->m_curtime)
		{
			Vector screen;

			if (!math::world_to_screen(damage_marker[i].position, screen))
				continue;

			auto alpha = (int)((damage_marker[i].hurt_time + 2.0f - m_globals()->m_curtime) * 127.f);
			damage_marker[i].hurt_color.SetAlpha(alpha);
			damage_marker[i].position.z -= damage_marker[i].positiony;
			render::get().text(fonts[NAME], screen.x, screen.y, damage_marker[i].hurt_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "-%i", damage_marker[i].damage);
		}
	}
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle(resolution + 2);

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0.0f;

	circle[0].rhw = 1.0f;
	circle[0].color = color2;

	for (auto i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - radius * sin(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0.0f;

		circle[i].rhw = 1.0f;
		circle[i].color = color;
	}

	device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX2), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr); //-V107

	if (!g_pVB2)
		return;

	void* pVertices;

	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX2), (void**)&pVertices, 0); //-V107
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX2));
	g_pVB2->Unlock();

	device->SetTexture(0, nullptr);
	device->SetPixelShader(nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX2));
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	g_pVB2->Release();
}

void otheresp::automatic_peek_indicator()
{
	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!g_ctx.globals.start_position.IsZero())
		position = g_ctx.globals.start_position;

	if (position.IsZero())
		return;

	static auto alpha = 0.0f;

	if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18) || alpha)
	{
		if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18))
			alpha += 3.0f * m_globals()->m_frametime; //-V807
		else
			alpha -= 3.0f * m_globals()->m_frametime;

		Vector origin = g_ctx.local()->m_vecOrigin();

		alpha = math::clamp(alpha, 0.0f, 1.0f);
		render::get().Draw3DCircleGradient(position, 25.f, Color(g_cfg.misc.automatic_peek2), Color(g_cfg.misc.automatic_peek2));

		Vector screen;
	}
}

void otheresp::DrawMolotov() {

	if (!g_cfg.esp.advance_mode)
		return;

	for (auto i = 0; i < m_entitylist()->GetHighestEntityIndex(); i++)
	{
		auto ent = m_entitylist()->GetClientEntity(i);
		if (!ent)
			continue;

		if (ent->GetClientClass()->m_ClassID != 100)
			continue;

		auto inferno = reinterpret_cast<inferno_t*>(ent);

		auto origin = inferno->m_vecOrigin();
		auto screen_origin = Vector();

		if (!math::world_to_screen(origin, screen_origin))
			return;

		const auto spawn_time = inferno->get_spawn_time();
		const auto timer = (spawn_time + inferno_t::get_expiry_time()) - m_globals()->m_curtime;
		const auto factor = timer / inferno_t::get_expiry_time();
		const auto l_spawn_time = *(float*)(uintptr_t(inferno) + 0x20);
		const auto l_factor = ((l_spawn_time + 7.03125f) - m_globals()->m_curtime) / 7.03125f;

		static auto size = Vector2D(35.0f, 5.0f);

		if (ent->GetClientClass()->m_ClassID == CInferno) {

			Vector mins, maxs;
			inferno->GetClientRenderable()->GetRenderBounds(mins, maxs);

			render::get().Draw3DCircleNoResize(inferno->m_vecOrigin(), 180.0f, Color(200, 30, 30));

		}

		Vector eye_pos{ };
		eye_pos = g_ctx.local()->m_vecOrigin() + g_ctx.local()->m_vecViewOffset();

		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (eye_pos.empty())
			return;

		auto dist_in_ft = [](Vector o, Vector dest)
		{
			Vector yo = Vector(dest.x - o.x, dest.y - o.y, dest.z - o.z);
			return std::roundf(std::sqrt(yo.x * yo.x + yo.y * yo.y + yo.z * yo.z));
		};

		auto factor1 = (spawn_time + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();

		auto local_dist = dist_in_ft(eye_pos, e->GetRenderOrigin());
		int m_dist = (int)local_dist;

		int redux = local_dist > 99 ? 11 : 9;

		//render::get().circle_filled(screen_origin.x, screen_origin.y, 60, 28, Color(0, 0, 0, 240));
		//render::get().text(fonts[WARNING], screen_origin.x - 4, screen_origin.y - 22, Color(255, 255, 255, 255), FONTFLAG_DROPSHADOW, "!");
		////render::get().text(fonts[ESP3], nadeEnd.x - 12, nadeEnd.y + 10, Color(255, 255, 255, 255), FONTFLAG_DROPSHADOW, "%i FT",

		//if (ent->GetClientClass()->m_ClassID == 100)
		//	draw_arc(screen_origin.x, screen_origin.y, 26, 28, -90, 360 * factor1, Color((232, 232, 232, 200)));
		//render::get().text("l", screen_origin.x - 8, screen_origin.y - 45.f - 20, 26.f, Color(255, 255, 255));

	}

}

std::vector<damage_indicator_t> dmg_indicator;