// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "antiaim.h"
#include "knifebot.h"
#include "zeusbot.h"
#include "..\misc\fakelag.h"
#include "..\misc\prediction_system.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\local_animations.h"

void antiaim::create_move(CUserCmd* m_pcmd)
{
	if (!g_cfg.ragebot.enable)
		return;

	if (condition(m_pcmd))
		return;

	m_pcmd->m_viewangles.x = get_pitch(m_pcmd);

	m_pcmd->m_viewangles.y = get_yaw(m_pcmd);
}

void VectorAngles1(const Vector& forward, QAngle& angles)
{
	if (forward[1] == 0.0f && forward[0] == 0.0f)
	{
		angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f;
		angles[1] = 0.0f;
	}
	else
	{
		angles[0] = atan2(-forward[2], forward.Length2D()) * -180 / M_PI;
		angles[1] = atan2(forward[1], forward[0]) * 180 / M_PI;

		if (angles[1] > 90) angles[1] -= 180;
		else if (angles[1] < 90) angles[1] += 180;
		else if (angles[1] == 90) angles[1] = 0;
	}

	angles[2] = 0.0f;
}

//void antiaim::fix_movement(CUserCmd* cmd)
//{
//	//get our viewangles for calculations for lator on
//	Vector real_angles;
//    m_engine()->GetViewAngles(real_angles);
//
//	// adjust for roll if we in nospread, no spread is dead but may aswell do this anyway
//	if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND) && real_angles.z != 0.f)
//		cmd->m_sidemove = 0.f;
//
//	//get our speed from cmd move crap
//	Vector m_flMove = Vector{ cmd->m_forwardmove, cmd->m_sidemove, cmd->m_upmove };
//	const float m_flVelocity = m_flMove.Length2D();
//
//	//no need fix if we not moving
//	if (m_flVelocity == 0.f)
//		return;
//
//	//calc our move angles
//	QAngle move_angle;
//	VectorAngles1(m_flMove, move_angle);
//
//	//get our needed yaw
//	float yaw = DEG2RAD(cmd->m_viewangles.y - real_angles.y + move_angle.yaw);
//
//	//fix our movements with the crap we calculated before
//	cmd->m_forwardmove = cos(yaw) * m_flVelocity;
//	cmd->m_sidemove = sin(yaw) * m_flVelocity;
//
//	//yeah this is a shit way fix leg slide but simple asf u can just do a hook to fix it or something
//	//fix our feet animations, doing it here cuz we don't really need fix it if we dont move
//	if (g_ctx.local()->get_move_type() != MOVETYPE_LADDER)
//		cmd->m_buttons &= ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);
//}

float antiaim::get_pitch(CUserCmd* m_pcmd)
{
	if (g_cfg.antiaim.pitch0land && g_ctx.local()->get_animation_state()->m_bInHitGroundAnimation && g_ctx.local()->get_animation_state()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
		return 0.0f;

	auto pitch = m_pcmd->m_viewangles.x;

	switch (g_cfg.antiaim.pitch)
	{
	case 0:
		pitch = 0.f;
		break;
	case 1:
		pitch = g_cfg.misc.anti_untrusted ? 89.f : 540.0f;
		break;
	case 2:
		pitch -= g_cfg.misc.anti_untrusted ? 89.f : 540.0f;
		break;
	}

	return pitch;
}

float antiaim::get_yaw(CUserCmd* m_pcmd)
{
	static auto invert_jitter = false;
	static auto should_invert = false;

	if (g_ctx.send_packet)
		should_invert = true;
	else if (!g_ctx.send_packet && should_invert) //-V560
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}

	auto max_desync_delta = g_ctx.local()->get_max_desync_delta(); //-V807

	auto yaw = 0.0f;
	auto lby_type = 0;

	if (g_cfg.antiaim.base_angle == 1 && manual_side == SIDE_NONE)
		freestanding(m_pcmd);
	else
		final_manual_side = manual_side;

	auto base_angle = m_pcmd->m_viewangles.y + 180.0f;

	if (final_manual_side == SIDE_LEFT)
		base_angle -= 90.0f;
	if (final_manual_side == SIDE_RIGHT)
		base_angle += 90.0f;

	if (g_cfg.antiaim.base_angle == 2 && manual_side == SIDE_NONE)
		base_angle = at_targets();

	if (g_cfg.antiaim.desync != 2 && (g_cfg.antiaim.flip_desync.key <= KEY_NONE || g_cfg.antiaim.flip_desync.key >= KEY_MAX))
	{
		if (g_cfg.antiaim.desync_dir == 0) {
			if (final_manual_side == SIDE_LEFT)
				flip = true;
			else if (final_manual_side == SIDE_RIGHT)
				flip = false;
		}
		else if (g_cfg.antiaim.desync_dir == 1)
			flip = automatic_direction();
	}
	else if (g_cfg.antiaim.desync == 1) {
		if (g_cfg.antiaim.desync_dir == 0)
			flip = key_binds::get().get_key_bind_state(16);
		else if (g_cfg.antiaim.desync_dir == 1)
			flip = automatic_direction();
	}

	auto yaw_angle = 0.0f;

	switch (g_cfg.antiaim.yaw)
	{
	case 1:
		base_angle += (invert_jitter  ? -0.5f : 0.5f) * (float)g_cfg.antiaim.range;
		break;
	case 2:
	/*	if (flip)
		{
			auto start_angle = (float)g_cfg.antiaim.range * 0.5f;
			auto end_angle = (float)g_cfg.antiaim.range * -0.5f;

			static auto angle = start_angle;

			auto angle_add_amount = (float)g_cfg.antiaim.speed * 0.5f;

			if (angle - angle_add_amount >= end_angle)
				angle -= angle_add_amount;
			else
				angle = start_angle;

			yaw_angle = angle;
		}
		else
		{
			auto start_angle = (float)g_cfg.antiaim.range * -0.5f;
			auto end_angle = (float)g_cfg.antiaim.range * 0.5f;

			static auto angle = start_angle;

			auto angle_add_amount = (float)g_cfg.antiaim.speed * 0.5f;

			if (angle + angle_add_amount <= end_angle)
				angle += angle_add_amount;
			else
				angle = start_angle;

			yaw_angle = angle;
		}*/
		base_angle = (base_angle - (float)g_cfg.antiaim.range / 2.f) + std::fmod(m_globals()->m_curtime * ((float)g_cfg.antiaim.speed * 20.f), (float)g_cfg.antiaim.range);
	break;
	}
	desync_angle = 0.0f;

	if (g_cfg.antiaim.desync)
	{
		if (g_cfg.antiaim.desync == 2)
			flip = invert_jitter;

		auto desync_delta = max_desync_delta;

		if (g_cfg.antiaim.lby_type)
			desync_delta *= 2.0f;
		else
			desync_delta = min(desync_delta, (float)g_cfg.antiaim.desync_range);

		if (!flip)
		{
			desync_delta = -desync_delta;
			max_desync_delta = -max_desync_delta;
		}

		base_angle -= desync_delta;

		if (!g_cfg.antiaim.lby_type)
		{
			if (!flip)
				base_angle += desync_delta * (float)g_cfg.antiaim.body_lean * 0.01f;
			else
				base_angle += desync_delta * (float)g_cfg.antiaim.inverted_body_lean * 0.01f;
		}

		desync_angle = desync_delta;
	}

	yaw = base_angle;

	if (!desync_angle) //-V550
		return yaw;

	lby_type = g_cfg.antiaim.lby_type;

	static auto sway_counter = 0;
	static auto force_choke = false;

	if (should_break_lby(m_pcmd, lby_type))
	{
		auto speed = 1.01f;

		if (m_pcmd->m_buttons & IN_DUCK || g_ctx.globals.fakeducking)
			speed *= 2.94117647f;

		static auto switch_move = false;

		if (switch_move)
			m_pcmd->m_sidemove += speed;
		else
			m_pcmd->m_sidemove -= speed;

		switch_move = !switch_move;

		if (lby_type != 2 || sway_counter > 3)
		{
			if (desync_angle > 0.0f)
				yaw -= 179.0f;
			else
				yaw += 179.0f;
		}

		if (sway_counter < 8)
			++sway_counter;
		else
			sway_counter = 0;

		breaking_lby = true;
		force_choke = true;
		g_ctx.send_packet = false;

		return yaw;
	}
	else if (force_choke)
	{
		force_choke = false;
		g_ctx.send_packet = false;

		return yaw;
	}
	else if (g_ctx.send_packet)
		yaw += desync_angle;

	return yaw;
}

bool antiaim::condition(CUserCmd* m_pcmd, bool dynamic_check)
{
	if (!m_pcmd)
		return true;

	if (!g_ctx.available())
		return true;

	if (!g_cfg.antiaim.enable)
		return true;

	if (!g_ctx.local()->is_alive()) //-V807
		return true;

	if (g_ctx.local()->m_bGunGameImmunity() || g_ctx.local()->m_fFlags() & FL_FROZEN)
		return true;

	if (g_ctx.local()->get_move_type() == MOVETYPE_NOCLIP || g_ctx.local()->get_move_type() == MOVETYPE_LADDER)
		return true;

	if (g_ctx.globals.aimbot_working)
		return true;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return true;

	if (m_pcmd->m_buttons & IN_ATTACK && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_non_aim())
		return true;

	auto revolver_shoot = weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !g_ctx.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

	if (revolver_shoot)
		return true;

	if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && weapon->is_knife())
		return true;

	if (dynamic_check && freeze_check)
		return true;

	if (dynamic_check && m_pcmd->m_buttons & IN_USE)
		return true;

	if (dynamic_check && weapon->is_grenade() && weapon->m_fThrowTime())
		return true;

	return false;
}

int antiaim::getTickBase(CUserCmd* ucmd) {

	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;

	if (!ucmd)
		return g_tick;

	if (!g_pLastCmd || g_pLastCmd->m_predicted) {
		g_tick = g_ctx.local()->m_nTickBase();
	}
	else {
		// Required because prediction only runs on frames, not ticks
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick
		++g_tick;
	}

	g_pLastCmd = ucmd;
	return g_tick;
}

bool antiaim::should_break_lby(CUserCmd* m_pcmd, int lby_type)
{
	if (!lby_type)
		return false;

	if (g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 12)
		return false;

	if (!g_ctx.globals.fakeducking && m_clientstate()->iChokedCommands > 14)
	{
		g_ctx.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	auto animstate = g_ctx.local()->get_animation_state(); //-V807

	if (!animstate)
		return false;

	if (animstate->m_velocity > 0.1f || fabs(animstate->flUpVelocity) > 100.0f)
		g_ctx.globals.next_lby_update = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase + 14);
	else
	{
		if (TICKS_TO_TIME(g_ctx.globals.fixed_tickbase) > g_ctx.globals.next_lby_update)
		{
			g_ctx.globals.next_lby_update = 0.0f;
			return true;
		}
	}

	return false;
}

float antiaim::at_targets()
{
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		auto weapon = e->m_hActiveWeapon().Get();

		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		auto fov = math::get_fov(angles, math::calculate_angle(g_ctx.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}

	auto angle = 180.0f;

	if (manual_side == SIDE_LEFT)
		angle = 90.0f;
	else if (manual_side == SIDE_RIGHT)
		angle = -90.0f;

	if (!target)
		return g_ctx.get_command()->m_viewangles.y + angle;

	return math::calculate_angle(g_ctx.globals.eye_pos, target->GetAbsOrigin()).y + angle;
}

bool antiaim::automatic_direction()
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = g_ctx.local();
	src3D = g_ctx.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;

	if (Left - Right > 10.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Right - Left > 10.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (right_ticks > 10)
		return true;
	else if (left_ticks > 10)
		return false;

	return flip;
}

void antiaim::freestanding(CUserCmd* m_pcmd)
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	trace_t tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = g_ctx.local();
	src3D = g_ctx.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	g_ctx.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	g_ctx.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;
	static auto back_ticks = 0;

	if (Right - Left > 20.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Left - Right > 20.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (fabs(Right - Left) <= 20.0f)
		back_ticks++;
	else
		back_ticks = 0;

	if (right_ticks > 10)
		final_manual_side = SIDE_RIGHT;
	else if (left_ticks > 10)
		final_manual_side = SIDE_LEFT;
	else if (back_ticks > 10)
		final_manual_side = SIDE_BACK;
}