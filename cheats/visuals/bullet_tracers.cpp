// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "bullet_tracers.h"
#include "..\..\sdk\misc\BeamInfo_t.hpp"
#include "..\ragebot\aim.h"
#include "..\..\utils\ctx.hpp"
#include "..\misc\logs.h"

void bullettracers::draw_beam(bool local_tracer, const Vector& src, const Vector& end, Color color)
{
	if (src == ZERO)
		return;

	BeamInfo_t beam_info;
	beam_info.m_vecStart = src;

	if (local_tracer)
		beam_info.m_vecStart.z -= 2.0f;

	beam_info.m_vecEnd = end;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = crypt_str("sprites/purplelaser1.vmt");
	beam_info.m_nModelIndex = -1;
	beam_info.m_flHaloScale = 0.0f;
	beam_info.m_flLife = 4.0f;
	beam_info.m_flWidth = 2.0f;
	beam_info.m_flEndWidth = 2.0f;
	beam_info.m_flFadeLength = 0.0f;
	beam_info.m_flAmplitude = 2.0f;
	beam_info.m_flBrightness = (float)color.a();
	beam_info.m_flSpeed = 0.2f;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0.0f;
	beam_info.m_flRed = (float)color.r();
	beam_info.m_flGreen = (float)color.g();
	beam_info.m_flBlue = (float)color.b();
	beam_info.m_nSegments = 2;
	beam_info.m_bRenderable = true;
	beam_info.m_nFlags = FBEAM_SHADEIN | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

	if (beam)
		m_viewrenderbeams()->DrawBeam(beam);
}	

void bullettracers::events(IGameEvent* event)
{
	auto event_name = event->GetName();

	if (!strcmp(event_name, crypt_str("bullet_impact")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id);

		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(user));

		if (e->valid(false))
		{
			if (e == g_ctx.local())
			{
				auto new_record = true;
				Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

				if (g_cfg.esp.hitmarker2)
					m_debugoverlay()->AddTextOverlayRGB(position, 0, 4.f, 255, 255, 255, 255, "+");
					//render::get().text(fonts[ESP], position.x, position.y, Color(255,255,255,255),

				for (auto& current : impacts)
				{
					if (e == current.e)
					{
						new_record = false;

						current.impact_position = position;
						current.time = m_globals()->m_curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						impact_data
						{
						e,
						position,
						m_globals()->m_curtime
						});
			}
			else if (e->m_iTeamNum() != g_ctx.local()->m_iTeamNum())
			{
				auto new_record = true;
				Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

				for (auto& current : impacts)
				{
					if (e == current.e)
					{
						new_record = false;

						current.impact_position = position;
						current.time = m_globals()->m_curtime;
					}
				}

				if (new_record)
					impacts.push_back(
						impact_data
						{
						e,
						position,
						m_globals()->m_curtime
						});
			}
		}
	}
}

void bullettracers::draw_beams()
{
	if (impacts.empty())
		return;

	while (!impacts.empty())
	{
		if (impacts.begin()->impact_position.IsZero())
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (fabs(m_globals()->m_curtime - impacts.begin()->time) > 4.0f)
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (!impacts.begin()->e->valid(false))
		{
			impacts.erase(impacts.begin());
			continue;
		}

		if (TIME_TO_TICKS(m_globals()->m_curtime) > TIME_TO_TICKS(impacts.begin()->time))
		{
			auto color = g_cfg.esp.enemy_bullet_tracer_color;

			if (impacts.begin()->e == g_ctx.local())
			{
				if (!g_cfg.esp.bullet_tracer)
				{
					impacts.erase(impacts.begin());
					continue;
				}

				color = g_cfg.esp.bullet_tracer_color;
			}
			else if (!g_cfg.esp.enemy_bullet_tracer)
			{
				impacts.erase(impacts.begin());
				continue;
			}

			// @note - если хочеш деф то раскоменти draw_beam и закоменти m_debugoverlay()->AddLineOverlayAlpha.
			m_debugoverlay()->AddLineOverlayAlpha(impacts.begin()->e == g_ctx.local() ? aim::get().last_shoot_position : impacts.begin()->e->get_shoot_position(), impacts.begin()->impact_position, (float)color.r(), (float)color.g(), (float)color.b(), 255, true, 4);
			//draw_beam(impacts.begin()->e == g_ctx.local(), impacts.begin()->e == g_ctx.local() ? aim::get().last_shoot_position : impacts.begin()->e->get_shoot_position(), impacts.begin()->impact_position, color);
			impacts.erase(impacts.begin());
			continue;
		}

		break;
	}
}

void bullettracers::draw_grenade_beam1(const Vector& src, const Vector& end, Color color)
{
	if (src == ZERO)
		return;

	BeamInfo_t beam_info;
	beam_info.m_vecStart = src;

	beam_info.m_vecEnd = end;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = crypt_str("sprites/lasebeam.vmt");
	beam_info.m_nModelIndex = -1;
	beam_info.m_flHaloScale = 0.0f;
	beam_info.m_flLife = 1.5f;
	beam_info.m_flWidth = 0.5f;
	beam_info.m_flEndWidth = 0.5f;
	beam_info.m_flFadeLength = 0.0f;
	beam_info.m_flAmplitude = 0.2f;
	beam_info.m_flBrightness = (float)color.a();
	beam_info.m_flSpeed = 0.5f;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0.0f;
	beam_info.m_flRed = (float)color.r();
	beam_info.m_flGreen = (float)color.g();
	beam_info.m_flBlue = (float)color.b();
	beam_info.m_nSegments = 2;
	beam_info.m_bRenderable = true;
	beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM; // FBEAM_SHADEIN FBEAM_ONLYNOISEONCE | FBEAM_NOTILE |

	auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

	if (beam)
		m_viewrenderbeams()->DrawBeam(beam);

	if (beam)
		m_cvar()->ConsolePrintf(crypt_str("ok"));
	else
		m_cvar()->ConsolePrintf(crypt_str("lol"));
}

void bullettracers::DrawBeamPaw(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/bubble.vmt";
	beamInfo.m_pszHaloName = "sprites/bubble.vmt";
	beamInfo.m_flHaloScale = 1.0;
	beamInfo.m_flWidth = 1.4f;
	beamInfo.m_flEndWidth = 1.4f;
	beamInfo.m_flFadeLength = 0.2f;
	beamInfo.m_flAmplitude = 2;
	beamInfo.m_flBrightness = float(220);
	beamInfo.m_flSpeed = 0.001f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.05;
	beamInfo.m_nFlags = FBEAM_NOTILE; //FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM
	Beam_t* myBeam = m_viewrenderbeams()->CreateBeamPoints(beamInfo);
	if (myBeam)
		m_viewrenderbeams()->DrawBeam(myBeam);
	// beamInfo.m_pszModelName = "sprites/radio.vmt";// рисуется за стеной
}

void bullettracers::DrawBeamPawWall(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/light_glow02_add_noz.vmt";
	beamInfo.m_pszHaloName = "sprites/light_glow02_add_noz.vmt";
	beamInfo.m_flHaloScale = 1.0;
	beamInfo.m_flWidth = 1.4f;
	beamInfo.m_flEndWidth = 1.4f;
	beamInfo.m_flFadeLength = 0.2f;
	beamInfo.m_flAmplitude = 2;
	beamInfo.m_flBrightness = float(180);
	beamInfo.m_flSpeed = 0.001f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.05;
	beamInfo.m_nFlags = FBEAM_NOTILE;
	Beam_t* myBeam = m_viewrenderbeams()->CreateBeamPoints(beamInfo);
	if (myBeam)
		m_viewrenderbeams()->DrawBeam(myBeam);
}

void bullettracers::draw_grenade_beam(const Vector& src, const Vector& end, Color color)
{

	if (src == ZERO)
		return;

	BeamInfo_t beam_info;
	beam_info.m_vecStart = src;

	beam_info.m_vecEnd = end;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = crypt_str("sprites/purplelaser1.vmt");
	beam_info.m_nModelIndex = -1;
	beam_info.m_flHaloScale = 0.0f;
	beam_info.m_flLife = 0.10f;
	beam_info.m_flWidth = 2.5f;
	beam_info.m_flEndWidth = 2.5f;
	beam_info.m_flFadeLength = 0.0f;
	beam_info.m_flAmplitude = 2.0f;
	beam_info.m_flBrightness = color.a(); //  / 10.f
	beam_info.m_flSpeed = 0.2f;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0.0f;
	beam_info.m_flRed = color.r();
	beam_info.m_flGreen = color.g();
	beam_info.m_flBlue = color.b();
	beam_info.m_nSegments = 2;
	beam_info.m_bRenderable = true;
	beam_info.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;

	auto beam = m_viewrenderbeams()->CreateBeamPoints(beam_info);

	if (beam)
		m_viewrenderbeams()->DrawBeam(beam);
}

void bullettracers::AddTrails()
{
	if (g_cfg.esp.trails)

		if (g_ctx.local() && g_ctx.local()->is_alive() && m_engine()->IsInGame() && m_engine()->IsConnected())
		{
			static float rainbow;
			rainbow += 0.001f;
			if (rainbow > 1.f)
				rainbow = 0.f;

			auto rainbow_col = Color::FromHSB(rainbow, 1, 1);
			auto local_pos = g_ctx.local()->m_vecOrigin();
			DrawBeamPaw(local_pos, Vector(local_pos.x, local_pos.y + 10, local_pos.z), rainbow_col);
		}
}