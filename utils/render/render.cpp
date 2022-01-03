#include "render.h"
#include <mutex>
#include "..\..\cheats\menu.h"

#include "../../cheats/visuals/other_esp.h"
#include "../../cheats/visuals/world_esp.h"
#include "../../cheats/visuals/player_esp.h"
#include "../../cheats/visuals/GrenadePrediction.h"
#include "../../cheats/visuals/bullet_tracers.h"
#include "../../cheats/misc/misc.h"
#include "../../cheats/misc/logs.h"
#include "../../cheats/lagcompensation/local_animations.h"

namespace fonts
{
	ImFont* Verdana;
	ImFont* Callibri;
	ImFont* Smallfonts;
	ImFont* Undefeated;
	ImFont* SmallestPixel;
	ImFont* IcoMoon;
	ImFont* Lucida;
	ImFont* Tahoma;
	ImFont* ZeusDildo;
}

namespace render_new {
	ImDrawData draw_data;

	Vector2D get_text_size(std::string_view txt, ImFont* font) {
		if (!font
			|| txt.empty()
			|| !font->IsLoaded())
			return Vector2D();

		const auto size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.f, txt.data());

		return Vector2D(IM_FLOOR(size.x + 0.95f), size.y);
	}

	void procces_render() { 
		m_draw_list = ImGui::GetBackgroundDrawList();

		for (auto a : old_proccesing)
		{
			auto col1 = a.color1;
			auto col2 = a.color2;
			auto col3 = a.color3;
			auto col4 = a.color4;

			switch (a.type) {
			case R_CIRCLE:
				m_draw_list->AddCircle(a.pos, a.radius, col1.hex(), a.points);

				break;

			case R_CIRCLE_FILLED:
				m_draw_list->AddCircleFilled(a.pos, a.radius, col1.hex(), a.points);

				break;

			case R_RECT:
				m_draw_list->AddRect(a.pos, a.size, col1.hex(), a.rounding);

				break;

			case R_RECT_FILLED:
				m_draw_list->AddRectFilled(a.pos, a.size, col1.hex(), a.rounding);

				break;

			case R_LINE:	
				m_draw_list->AddLine(a.pos, a.size, col1.hex());

				break;

			case R_GRADIENT:
				m_draw_list->AddRectFilledMultiColor(a.pos, a.pos + a.size, col1.hex(), col2.hex(), col3.hex(), col4.hex());
			
				break;

			case R_POLYGON:
				m_draw_list->_Path.reserve(m_draw_list->_Path.Size + a.poly_points.size() + 1);

				for (auto& point : a.poly_points) {
					m_draw_list->_Path.push_back(*reinterpret_cast<const ImVec2*>(&point));
				}

				m_draw_list->PathStroke(col1.hex(), true, 1.f);
				
				break;

			case R_POLYGON_FILLED:
				m_draw_list->_Path.reserve(m_draw_list->_Path.Size + a.poly_points.size() + 1);

				for (auto& point : a.poly_points) {
					m_draw_list->_Path.push_back(*reinterpret_cast<const ImVec2*>(&point));
				}

				m_draw_list->PathFillConvex(col1.hex());

				break;

			case R_ARC:
				m_draw_list->PathArcTo(a.pos, a.radius, DEG2RAD(a.min_angle), DEG2RAD(a.max_angle), 32);
				m_draw_list->PathStroke(col1.hex(), false, a.thickness);

				break;

			case R_TEXT:
				m_draw_list->AddText(a.font, a.font_size, a.pos, col1.hex(), a.text.data());
				break;

			case R_TEXT_OUTLINE:
				m_draw_list->AddTextOutline(a.font, a.font_size, a.pos, col1.hex(), a.text.data());
				break;

			case R_TEXT_SHADOW:
				m_draw_list->AddTextSoftShadow(a.font, a.font_size, a.pos, col1.hex(), a.text.data());
				break;

			default:
				continue;
			}
			
		}
	}

	void text(std::string_view txt, int font_size, Vector2D pos, const Color& clr, ImFont* font, bit_flag_t<uint8_t> flags) {
		if (!font)
			return;

		const auto centered_x = flags.has(FONT_CENTERED_X);
		const auto centered_y = flags.has(FONT_CENTERED_Y);

		if (centered_x
			|| centered_y) {
			const auto text_size = get_text_size(txt, font);

			if (centered_x) {
				pos.x -= text_size.x / 2.f;
			}

			if (centered_y) {
				pos.y -= text_size.y / 2.f;
			}
		}

		bool shadow = flags.has(FONT_DROP_SHADOW);
		bool outline = flags.has(FONT_OUTLINE);

		if (shadow || outline) {
			if (shadow) {
				rendering shadowed_text;

				shadowed_text.type = R_TEXT_SHADOW;
				shadowed_text.color1 = clr;
				shadowed_text.font_size = font_size;
				shadowed_text.font = font;
				shadowed_text.text = txt.data();
				shadowed_text.pos = *reinterpret_cast<const ImVec2*>(&pos);

				proccesing.emplace_back(shadowed_text);
			}

			
			if (outline) {
				rendering outlined_text;

				outlined_text.type = R_TEXT_OUTLINE;
				outlined_text.color1 = clr;
				outlined_text.font_size = font_size;
				outlined_text.font = font;
				outlined_text.text = txt.data();
				outlined_text.pos = *reinterpret_cast<const ImVec2*>(&pos);

				proccesing.emplace_back(outlined_text);
			}
		}
		else {
			rendering text;

			text.type = R_TEXT;
			text.color1 = clr;
			text.font_size = font_size;
			text.font = font;
			text.text = txt.data();
			text.pos = *reinterpret_cast<const ImVec2*>(&pos);

			proccesing.emplace_back(text);
		}
	}

	void line(const Vector2D& from, const Vector2D& to, const Color& clr) {
		rendering line;

		line.type = R_LINE;
		line.color1 = clr;
		line.pos = *reinterpret_cast<const ImVec2*>(&from);
		line.size = *reinterpret_cast<const ImVec2*>(&to);

		proccesing.emplace_back(line);
	}

	void rect(const Vector2D& pos, const Vector2D& size, const Color& clr, float rounding) {
		rendering rect;

		rect.type = R_RECT;
		rect.color1 = clr;
		rect.pos = *reinterpret_cast<const ImVec2*>(&pos);
		rect.size = ImVec2(pos.x + size.x, pos.y + size.y);
		rect.rounding = rounding;
		
		proccesing.emplace_back(rect);
	}

	void rect_filled(const Vector2D& pos, const Vector2D& size, const Color& clr, float rounding) {
		rendering rect;

		rect.type = R_RECT_FILLED;
		rect.color1 = clr;
		rect.pos = *reinterpret_cast<const ImVec2*>(&pos);
		rect.size = ImVec2(pos.x + size.x, pos.y + size.y);
		rect.rounding = rounding;

		proccesing.emplace_back(rect);
	}

	void rect_filed_multi_clr(int x, int y, int w, int h, const Color& clr_upr_left, const Color& clr_upr_right, const Color& clr_bot_left, const Color& clr_bot_right) {
		rendering rect;

		rect.type = R_GRADIENT;

		rect.color1 = clr_upr_left;
		rect.color2 = clr_upr_right;
		rect.color3 = clr_bot_left;
		rect.color4 = clr_bot_right;

		rect.pos = ImVec2(x, y);
		rect.size = ImVec2(w, h);

		proccesing.emplace_back(rect);
	}

	void begin() {
		if (g_ctx.available())
		{
			g_ctx.globals.bomb_carrier = -1;

			worldesp::get().paint_traverse();
			playeresp::get().paint_traverse();

			misc::get().desync_arrows();
			misc::get().zeus_range();

			auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

			if (weapon->is_grenade() && g_cfg.esp.grenade_prediction && g_cfg.player.enable)
				GrenadePrediction::get().Paint();

			static float anim1 = 20;

			if (!g_ctx.local()->m_bIsScoped())
				anim1 = 20;

			if (g_cfg.player.enable && g_cfg.esp.removals[REMOVALS_SCOPE] && g_ctx.globals.scoped && weapon->is_sniper())
			{
				static int w, h;
				m_engine()->GetScreenSize(w, h);

				static int cx = w / 2, cy = h / 2;

				anim1 += m_globals()->m_frametime * 500;

				if (anim1 >= 130)
					anim1 = 130;

				if (g_cfg.esp.fade_scope) {
					render::get().gradient(cx - g_cfg.esp.gap_sniper - 20 - 60 - g_cfg.esp.size_sniper + anim1, cy, 60 + g_cfg.esp.size_sniper, 1, Color(g_cfg.esp.fade2), Color(g_cfg.esp.fade1), GRADIENT_HORIZONTAL);
					render::get().gradient(cx + g_cfg.esp.gap_sniper + 20, cy, 60 + g_cfg.esp.size_sniper + anim1, 1, Color(g_cfg.esp.fade1), Color(g_cfg.esp.fade2), GRADIENT_HORIZONTAL);

					render::get().gradient(cx, cy - g_cfg.esp.gap_sniper - 20 - 60 - g_cfg.esp.size_sniper - anim1, 1, 60 + g_cfg.esp.size_sniper, Color(g_cfg.esp.fade2), Color(g_cfg.esp.fade1), GRADIENT_VERTICAL);
					render::get().gradient(cx, cy + g_cfg.esp.gap_sniper + 20, 1, 60 + g_cfg.esp.size_sniper - anim1, Color(g_cfg.esp.fade1), Color(g_cfg.esp.fade2), GRADIENT_VERTICAL);
				}
				else {
					render::get().line(w / 2, 0, w / 2, h, Color::Black);
					render::get().line(0, h / 2, w, h / 2, Color::Black);
				}
			}

			if (g_cfg.player.enable)
				otheresp::get().hitmarker_paint();

			if (g_cfg.player.enable && g_cfg.esp.damage_marker)
				otheresp::get().damage_marker_paint();


			otheresp::get().penetration_reticle();
			otheresp::get().automatic_peek_indicator();
			//otheresp::get().grenade1();
			//otheresp::get().grenade();

			//if (g_cfg.esp.esp_crosshair)
			//	worldesp::get().RenderCrosshair();

			otheresp::get().fake();

			//misc::get().ChatSpamer();
			misc::get().spectators_list();

			playeresp::get().centerinds();

			playeresp::get().anotaone();

			playeresp::get().evolveinds();

			bullettracers::get().draw_beams();
			bullettracers::get().AddTrails();

			if (g_cfg.esp.circlefake)
				otheresp::get().circlefake();

			if (g_cfg.esp.esp_molotov_timer);
			otheresp::get().DrawMolotov();
		}

		static auto framerate = 0.0f;
		framerate = 0.9f * framerate + 0.1f * m_globals()->m_absoluteframetime;

		if (framerate <= 0.0f)
			framerate = 1.0f;

		g_ctx.globals.framerate = (int)(1.0f / framerate);
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto latency = m_engine()->IsPlayingDemo() ? 0.0f : nci->GetAvgLatency(FLOW_OUTGOING);

			if (latency)
			{
				static auto cl_updaterate = m_cvar()->FindVar(crypt_str("cl_updaterate"));
				latency -= 0.5f / cl_updaterate->GetFloat();
			}

			g_ctx.globals.ping = (int)(max(0.0f, latency) * 1000.0f);
		}

		time_t lt;
		struct tm* t_m;

		lt = time(nullptr);
		t_m = localtime(&lt);

		auto time_h = t_m->tm_hour;
		auto time_m = t_m->tm_min;
		auto time_s = t_m->tm_sec;

		std::string time;

		if (time_h < 10)
			time += "0";

		time += std::to_string(time_h) + ":";

		if (time_m < 10)
			time += "0";

		time += std::to_string(time_m) + ":";

		if (time_s < 10)
			time += "0";

		time += std::to_string(time_s);
		g_ctx.globals.time = std::move(time);

		static int w, h;
		m_engine()->GetScreenSize(w, h);

		static auto alpha = 0;
		auto speed = 800.0f * m_globals()->m_frametime;

		misc::get().watermark();
		otheresp::get().draw_velocity();

		eventlogs::get().paint_traverse();

		misc::get().NightmodeFix();
		otheresp::get().indicators();
		if (g_ctx.globals.loaded_script)
			for (auto current : c_lua::get().hooks.getHooks(crypt_str("on_paint")))
				current.func();
		otheresp::get().draw_indicators();

		render_new::old_proccesing = render_new::proccesing;
	}

	void circle(int x, int y, int points, int radius, Color color)
	{
		rendering circle;

		circle.type = R_CIRCLE;
		circle.color1 = color;
		circle.pos = ImVec2(x,y);
		circle.points = points;
		circle.radius = radius;

		proccesing.emplace_back(circle);
	}

	void circle_filled(int x, int y, int points, int radius, Color color)
	{
		rendering circle;

		circle.type = R_CIRCLE_FILLED;
		circle.color1 = color;
		circle.pos = ImVec2(x, y);
		circle.points = points;
		circle.radius = radius;

		proccesing.emplace_back(circle);
	}

	void polygon(std::vector<Vector2D> points, const Color& clr) {
		rendering polygon;

		polygon.type = R_POLYGON;
		polygon.color1 = clr;
		polygon.poly_points = points;

		proccesing.emplace_back(polygon);
	}

	void polygon_filled(std::vector<Vector2D> points, const Color& clr) {
		rendering polygon;

		polygon.type = R_POLYGON_FILLED;
		polygon.color1 = clr;
		polygon.poly_points = points;

		proccesing.emplace_back(polygon);
	}

	void arc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness) {
		rendering arc;

		arc.type = R_ARC;
		arc.color1 = col;
		arc.pos = ImVec2(x, y);
		arc.radius = radius;
		arc.min_angle = min_angle;
		arc.max_angle = max_angle;
		arc.thickness = thickness;

		proccesing.emplace_back(arc);
	}

	ImDrawList* m_draw_list;
}