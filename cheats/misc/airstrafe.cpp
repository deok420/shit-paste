// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "airstrafe.h"
#include "..\misc\prediction_system.h"

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
DWORD m_strafe_flags;
float m_last_yaw;
__forceinline float GetYaw(float a1, float a2) {

	if (a1 != 0.f || a2 != 0.f)
		return atan2f(a2, a1) * 57.295776f;
	return 0.f;
}

void airstrafe::create_move(CUserCmd* m_pcmd) //-V2008
{
	if (g_ctx.local()->get_move_type() == MOVETYPE_LADDER) //-V807
		return;

	if (!g_cfg.misc.airstrafe)
		return;

	if (g_ctx.local()->m_fFlags() & FL_ONGROUND)
		return;

	Vector velocity = g_ctx.local()->m_vecVelocity();
	float velocity_len = g_ctx.local()->m_vecVelocity().Length2D();

	float strafer_smoothing = 70.f;
	float ideal_step = min(90.f, 845.5f / velocity_len);
	float velocity_yaw = GetYaw(velocity.x, velocity.y);

	Vector unmod_angles = m_pcmd->m_viewangles;
	Vector angles = m_pcmd->m_viewangles;

	if (velocity_len < 2.f && m_pcmd->m_buttons & IN_JUMP)
		m_pcmd->m_forwardmove = 450.f;

	float forward_move = m_pcmd->m_forwardmove;

	if (forward_move != 0.f || m_pcmd->m_sidemove != 0.f) {
		m_pcmd->m_forwardmove = 0.f;

		if (velocity_len != 0.f && fabsf(velocity.z) != 0.f) {
		DO_IT_AGAIN:
			Vector forw, right;
			math::angle_vectors(angles, &forw, &right, nullptr);

			float v262 = (forw.x * forward_move) + (m_pcmd->m_sidemove * right.x);
			float v263 = (right.y * m_pcmd->m_sidemove) + (forw.y * forward_move);
			angles.y = GetYaw(v262, v263);
		}
	}

	float yaw_to_use = 0.f;
	m_strafe_flags &= ~4;

	float clamped_angles = angles.y;
	if (clamped_angles < -180.f) clamped_angles += 360.f;
	if (clamped_angles > 180.f) clamped_angles -= 360.f;

	yaw_to_use = m_pcmd->m_viewangles.y;
	m_strafe_flags |= 4;
	m_last_yaw = clamped_angles;

	if (m_strafe_flags & 4) {
		float diff = angles.y - yaw_to_use;
		if (diff < -180.f) diff += 360.f;
		if (diff > 180.f) diff -= 360.f;

		if (fabsf(diff) > ideal_step && fabsf(diff) <= 30.f) {
			float move = 450.f;
			if (diff < 0.f)
				move *= -1.f;

			m_pcmd->m_sidemove = move;
			return;
		}
	}

	float diff = angles.y - velocity_yaw;
	if (diff < -180.f) diff += 360.f;
	if (diff > 180.f) diff -= 360.f;

	float step = ((100 - g_cfg.misc.strafe_smoothing) * 0.02f) * (ideal_step + ideal_step);
	float sidemove = 0.f;
	if (fabsf(diff) > 170.f && velocity_len > 80.f || diff > step && velocity_len > 80.f) {
		angles.y = step + velocity_yaw;
		m_pcmd->m_sidemove = -450.f;
	}
	else if (-step <= diff || velocity_len <= 80.f) {
		if (m_strafe_flags & 1) {
			angles.y -= ideal_step;
			m_pcmd->m_sidemove = -450.f;
		}
		else {
			angles.y += ideal_step;
			m_pcmd->m_sidemove = 450.f;
		}
	}
	else {
		angles.y = velocity_yaw - step;
		m_pcmd->m_sidemove = 450.f;
	}
	if (!(m_pcmd->m_buttons & 16) && m_pcmd->m_sidemove == 0.f)
		goto DO_IT_AGAIN;

	m_strafe_flags ^= (m_strafe_flags ^ ~m_strafe_flags) & 1;

	g_ctx.globals.wish_angle = angles;
}