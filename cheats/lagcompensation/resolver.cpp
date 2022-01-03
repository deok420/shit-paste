// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
	player = e;
	player_record = record;

	original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
	original_pitch = math::normalize_pitch(pitch);
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	side = false;
	fake = false;

	was_first_bruteforce = false;
	was_second_bruteforce = false;

	original_goal_feet_yaw = 0.0f;
	original_pitch = 0.0f;
}

void resolver::resolve_yaw()
{
	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(player->EntIndex(), &player_info))
		return;

	if (player_info.fakeplayer || !g_ctx.local()->is_alive() || player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
	{ // do we need to resolve player?
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// shit player?
	auto animstate = player->get_animation_state();

	if (!animstate)
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	//on the stairs or noclip antiaims dont' work
	if (player->get_move_type() == MOVETYPE_LADDER || player->get_move_type() == MOVETYPE_NOCLIP)
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// speed for later using
	auto speed = player->m_vecVelocity().Length2D();

	// some bools for history checks
	bool is_player_zero = false;
	bool is_player_faking = false;
	bool is_player_low = false;
	int positives = 0;
	int negatives = 0;

	// save history resolver type
	resolver_history res_history = HISTORY_UNKNOWN;

	// do we have record for that player in history?
	for (auto it = lagcompensation::get().player_sets.begin(); it != lagcompensation::get().player_sets.end(); ++it)
		if (it->id == player_info.steamID64)
		{
			// copy information about resolver history to vars
			res_history = it->res_type;
			is_player_faking = it->faking;
			positives = it->pos;
			negatives = it->neg;

			if (!is_player_low)
				is_player_low = (speed > 38.f && it->low_move) || (speed <= 38.f && it->low_stand);

			// we dont want to continue cycle any more
			break;
		}

	// ghetto way
	auto choked = TIME_TO_TICKS(player->m_flSimulationTime() - player->m_flOldSimulationTime());

	// if his pitch down, or he is choking or we already hitted in desync, we can say that he might use desync
	if (fabs(original_pitch) > 65.f || choked >= 1 || is_player_faking)
		fake = true;
	else if (!fake && !g_ctx.globals.missed_shots[player->EntIndex()])
	{
		player_record->side = RESOLVER_ORIGINAL;
		return;
	}

	// did we hit player in low or zero side?
	if (res_history == HISTORY_ZERO)
		is_player_zero = true;

	// did we hit a zero-desync ? stay zero : continue
	if (is_player_zero && !g_ctx.globals.missed_shots[player->EntIndex()])
	{
		player_record->side = RESOLVER_ZERO;
		player_record->type = resolver_type::HISTORY_SIDE;
		return;
	}

	//Vector current;
	//float back_two, right_two, left_two;
	//trace_t tr;
	//Ray_t ray, ray2, ray3;
	//CTraceFilter filter;

	//Vector right(player->m_angEyeAngles().x, player->m_angEyeAngles().y + player->get_max_desync_delta(), player->m_angEyeAngles().z);
	//Vector left(player->m_angEyeAngles().x, player->m_angEyeAngles().y - player->get_max_desync_delta(), player->m_angEyeAngles().z);
	//Vector back(player->m_angEyeAngles().x, 180.f, player->m_angEyeAngles().z);
	//current = player->m_angEyeAngles();

	//filter.pSkip = player;
	//float distance = /*whatever*/;
	//ray.Init(current, right);
	//ray2.Init(current, left);
	//ray3.Init(current, back);

	//float back_one, right_one, left_one;

	//right_one = current.y - right.y;
	//left_one = current.y - left.y;
	//back_one = current.y - back.y;

	//m_trace()->TraceRay(ray, MASK_SHOT, &filter, &tr);
	//right_two = tr.endpos.Length2D() - tr.startpos.Length2D();

	//m_trace()->TraceRay(ray2, MASK_SHOT, &filter, &tr);
	//left_two = tr.endpos.Length2D() - tr.startpos.Length2D();

	//m_trace()->TraceRay(ray3, MASK_SHOT, &filter, &tr);
	//back_two = tr.endpos.Length2D() - tr.startpos.Length2D();

	//auto records = &player_records[player->EntIndex()]; //-V826

	//if (records->empty())
	//	return;

	//auto record = &records->front();

	//float side = record->side;

	//side = 0;

	//// if extending we have an easier time finding their real
	//if (player->get_animlayers()[3].m_flCycle == 0.f && player->get_animlayers()[3].m_flCycle == 0.f)
	//{
	//	if (fabs(right_one) >= player->get_max_desync_delta())
	//		side = 1;
	//	else if (fabs(left_one) >= player->get_max_desync_delta())
	//		side = -1;
	//	else if (fabs(back_one) >= player->get_max_desync_delta())
	//		side = 0;
	//}
	//// else we use tracing
	//else
	//{
	//	if (fabs(right_two) >= distance)
	//		side = 1;
	//	else if (fabs(left_two) >= distance)
	//		side = -1;
	//	else if (fabs(back_two) >= distance)
	//		side = 0;
	//}

start_side_detect:

	bool was_low_detect = false;

	// onetap.com method for stand checks
	if (player_record->shot)
	{
		auto left_side_diff = fabsf(math::angle_difference(positive_side, gfy_default));
		auto right_side_diff = fabsf(math::angle_difference(negative_side, gfy_default));

		if (fabsf(left_side_diff - right_side_diff) > 30.f)
		{
			player_record->side = left_side_diff > right_side_diff ? RESOLVER_FIRST : RESOLVER_SECOND;
			player_record->type = resolver_type::SHOT;
		}
		else {
			player_record->side = RESOLVER_ORIGINAL;
			player_record->type = resolver_type::SHOT;
			return;
		}
	}
	else if (speed <= 1.1f && player_record->layers[3].m_flWeight == 0.f && player_record->layers[3].m_flCycle == 0.f)
	{
		auto m_delta = math::angle_difference(player->m_angEyeAngles().y, gfy_default);
		auto m_side = (2 * (m_delta <= 0.0f) - 1) > 0;

		player_record->side = m_side ? RESOLVER_FIRST : RESOLVER_SECOND;
		player_record->type = resolver_type::ANIM_s;

		lock_side = m_globals()->m_curtime + 0.8f;
	}
	else if (speed <= 1.1f && (player_record->layers[12].m_flPlaybackRate == 0.f || player_record->layers[3].m_flWeight > 0.1f && player_record->layers[3].m_nSequence == 4 && player_record->layers[6].m_flCycle < 0.9f)) {
		auto m_delta = math::angle_difference(player->m_angEyeAngles().y - player->m_flLowerBodyYawTarget(), 360.f) <= 0.f;

		//proper lby resolver from pidoria.uno
		if (2 * m_delta) {
			if (2 * m_delta == 2) {
				player_record->side = RESOLVER_SECOND;
				player_record->type = resolver_type::LBY;
			}
			else {
				player_record->side = RESOLVER_FIRST;
				player_record->type = resolver_type::LBY;
			}

			lock_side = m_globals()->m_curtime + 0.8f;
		}
		else
			goto res_label1;
	}
	else if (speed > 1.1f && (player_record->layers[6].m_flWeight * 1000.f) == (previous_layers[6].m_flWeight * 1000.f))
	{
		float delta_first = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[0][6].m_flPlaybackRate);
		float delta_second = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[2][6].m_flPlaybackRate);
		float delta_third = abs(player_record->layers[6].m_flPlaybackRate - resolver_layers[1][6].m_flPlaybackRate);

		if (delta_first < delta_second || delta_third <= delta_second || (delta_second * 1000.0))
		{
			if (delta_first >= delta_third && delta_second > delta_third && !(delta_third * 1000.0)) {
				player_record->side = RESOLVER_FIRST;
				player_record->type = resolver_type::ANIM_m;
			}
			else
				goto res_label1;
		}
		else {
			player_record->side = RESOLVER_SECOND;
			player_record->type = resolver_type::ANIM_m;
		}
	}
	else
	{
	res_label1:
		{
		freestand:
			if (m_globals()->m_curtime - lock_side > 2.0f)
			{
				// lets compare two sides
				auto fire_data_first = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.first), player);
				auto fire_data_second = autowall::get().wall_penetration(g_ctx.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.second), player);

				// fake head check
				if (fire_data_first.visible != fire_data_second.visible)
				{
					player_record->type = resolver_type::TRACE;
					player_record->side = fire_data_second.visible ? RESOLVER_FIRST : RESOLVER_SECOND;
					lock_side = m_globals()->m_curtime;
				}
				else
				{
					player_record->type = resolver_type::DIRECTIONAL;

					if (fire_data_first.damage != fire_data_second.damage)
						player_record->side = fire_data_first.damage < fire_data_second.damage ? RESOLVER_FIRST : RESOLVER_SECOND;
					else if (abs(positives - negatives) > 3)
					{
						player_record->side = positives > negatives ? RESOLVER_FIRST : RESOLVER_SECOND;
						player_record->type = resolver_type::HISTORY_SIDE;
					}
					else if (player_record->side <= 1)
						player_record->side = positives > negatives ? RESOLVER_FIRST : RESOLVER_SECOND;
				}
			}
			else
				player_record->type = resolver_type::LOCKED_SIDE;
		}
	}

	if (player_record->side == RESOLVER_FIRST && is_player_low)
		player_record->side = RESOLVER_LOW_FIRST;
	else if (player_record->side == RESOLVER_SECOND && is_player_low)
		player_record->side = RESOLVER_LOW_SECOND;

	side = (int)player_record->side == 2 || (int)player_record->side == 4;

	// bruteforce after miss, i think we have stored last side, yeah?
	if (g_ctx.globals.missed_shots[player->EntIndex()])
	{
		player_record->type = BRUTEFORCE;

		switch (last_side)
		{
		case RESOLVER_ORIGINAL:
			// did we missed in non-resolved side?

			// start bruteforce from zero and set static fake = true;
			g_ctx.globals.missed_shots[player->EntIndex()] = 0;
			fake = true;

			goto start_side_detect;
		case RESOLVER_ZERO:
			was_first_bruteforce = false;
			was_second_bruteforce = false;
			was_first_low_bruteforce = false;
			was_second_low_bruteforce = false;

			// continue bruteforce with detect side
			if (was_low_detect)
				player_record->side = side ? RESOLVER_LOW_FIRST : RESOLVER_LOW_SECOND;
			else
				player_record->side = side ? RESOLVER_FIRST : RESOLVER_SECOND;

			return;
		case RESOLVER_FIRST:
			player_record->side = was_second_bruteforce ? ((was_first_low_bruteforce && was_second_low_bruteforce) ? RESOLVER_ZERO : ((was_first_low_bruteforce && !was_second_low_bruteforce) ? RESOLVER_LOW_SECOND : ((!was_first_low_bruteforce && was_second_low_bruteforce) ? RESOLVER_LOW_FIRST : (side ? RESOLVER_LOW_FIRST : RESOLVER_LOW_SECOND)))) : RESOLVER_SECOND;

			was_first_bruteforce = true;
			return;
		case RESOLVER_SECOND:
			player_record->side = was_first_bruteforce ? ((was_first_low_bruteforce && was_second_low_bruteforce) ? RESOLVER_ZERO : ((was_first_low_bruteforce && !was_second_low_bruteforce) ? RESOLVER_LOW_SECOND : ((!was_first_low_bruteforce && was_second_low_bruteforce) ? RESOLVER_LOW_FIRST : (side ? RESOLVER_LOW_FIRST : RESOLVER_LOW_SECOND)))) : RESOLVER_FIRST;

			was_second_bruteforce = true;
			return;
		case RESOLVER_LOW_FIRST:
			player_record->side = was_second_low_bruteforce ? ((was_first_bruteforce && was_second_bruteforce) ? RESOLVER_ZERO : ((was_first_bruteforce && !was_second_bruteforce) ? RESOLVER_SECOND : ((!was_first_bruteforce && was_second_bruteforce) ? RESOLVER_FIRST : (side ? RESOLVER_FIRST : RESOLVER_SECOND)))) : RESOLVER_LOW_SECOND;

			was_first_low_bruteforce = true;
			return;
		case RESOLVER_LOW_SECOND:
			player_record->side = was_first_low_bruteforce ? ((was_first_bruteforce && was_second_bruteforce) ? RESOLVER_ZERO : ((was_first_bruteforce && !was_second_bruteforce) ? RESOLVER_SECOND : ((!was_first_bruteforce && was_second_bruteforce) ? RESOLVER_FIRST : (side ? RESOLVER_FIRST : RESOLVER_SECOND)))) : RESOLVER_LOW_FIRST;

			was_second_low_bruteforce = true;
			return;
		}
	}
}

float resolver::resolve_pitch()
{
	return original_pitch;
}