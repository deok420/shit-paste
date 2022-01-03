#include "..\..\includes.hpp"

class Vector;
class QAngle;
class CViewSetup;

#include "bullet_tracers.h"
#include "other_esp.h"

inline float CSGO_Armor1(float flDamage, int ArmorValue) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

struct c_grenade_prediction : public singleton<c_grenade_prediction> {
public:
	struct data_t {
	public:
		bool											m_detonated{};
		player_t* m_owner;
		Vector									m_origin{}, m_velocity{};
		entity_t* m_last_hit_entity{};
		Collision_Group_t				m_collision_group{};
		float											m_detonate_time{}, m_expire_time{};
		int												m_index{}, m_tick{}, m_next_think_tick{},
			m_last_update_tick{}, m_bounces_count{};
		std::vector< std::pair< Vector, bool > >	m_path{};

		__forceinline bool physics_simulate() {
			if (m_detonated)
				return true;

			static const auto sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));

			const auto new_velocity_z = m_velocity.z - (sv_gravity->GetFloat() * 0.4f) * m_globals()->m_intervalpertick;

			const auto move = Vector(
				m_velocity.x * m_globals()->m_intervalpertick,
				m_velocity.y * m_globals()->m_intervalpertick,
				(m_velocity.z + new_velocity_z) / 2.f * m_globals()->m_intervalpertick
			);

			m_velocity.z = new_velocity_z;

			auto trace = trace_t();

			physics_push_entity(move, trace);

			if (m_detonated)
				return true;

			if (trace.fraction != 1.f) {
				update_path< true >();

				perform_fly_collision_resolution(trace);
			}

			return false;
		}

		__forceinline void physics_trace_entity(Vector& src, Vector& dst, std::uint32_t mask, trace_t& trace) {
			m_trace()->trace_hull(
				src, dst,
				mask, trace
			);

			if (trace.startsolid
				&& (trace.contents & CONTENTS_CURRENT_90)) {
				trace.clear();

				m_trace()->trace_hull(
					src, dst,
					mask & ~CONTENTS_CURRENT_90, trace
				);
			}

			if (!trace.DidHit()
				|| !trace.hit_entity
				|| !((entity_t*)trace.hit_entity)->is_player())
				return;

			trace.clear();

			m_trace()->trace_line(src, dst, mask, &trace);
		}

		__forceinline void physics_push_entity(const Vector& push, trace_t& trace) {
			physics_trace_entity(m_origin, m_origin + push,
				m_collision_group == COLLISION_GROUP_DEBRIS
				? (CONTENTS_SOLID | CONTENTS_CURRENT_90) & ~CONTENTS_MONSTER
				: CONTENTS_SOLID | CONTENTS_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE | CONTENTS_CURRENT_90 | CONTENTS_HITBOX,
				trace
			);

			if (trace.startsolid) {
				m_collision_group = COLLISION_GROUP_INTERACTIVE_DEB;

				m_trace()->trace_line(
					m_origin - push, m_origin + push,
					(CONTENTS_SOLID | CONTENTS_CURRENT_90) & ~CONTENTS_MONSTER,
					&trace
				);
			}

			if (trace.fraction) {
				m_origin = trace.endpos;
			}

			if (!trace.hit_entity)
				return;

			if (((entity_t*)trace.hit_entity)->is_player()
				|| m_index != WEAPON_TAGRENADE && m_index != WEAPON_MOLOTOV && m_index != WEAPON_INCGRENADE)
				return;

			static const auto weapon_molotov_maxdetonateslope = m_cvar()->FindVar(crypt_str("weapon_molotov_maxdetonateslope"));

			if (m_index != WEAPON_TAGRENADE
				&& trace.plane.normal.z < (float)cos((float)DEG2RAD(weapon_molotov_maxdetonateslope->GetFloat())))
				return;

			detonate< true >();
		}

		__forceinline void perform_fly_collision_resolution(trace_t& trace) {
			auto surface_elasticity = 1.f;

			if (trace.hit_entity) {
				if (autowall::get().is_breakable_entity(trace.hit_entity)) {
				BREAKTROUGH:
					m_velocity *= 0.4f;

					return;
				}

				const auto is_player = ((entity_t*)trace.hit_entity)->is_player();
				if (is_player) {
					surface_elasticity = 0.3f;
				}

				if (trace.did_hit_non_world_entity()) {
					if (is_player
						&& m_last_hit_entity == trace.hit_entity) {
						m_collision_group = COLLISION_GROUP_DEBRIS;

						return;
					}

					m_last_hit_entity = (entity_t*)trace.hit_entity;
				}
			}

			auto velocity = ZERO;

			const auto back_off = m_velocity.Dot(trace.plane.normal) * 2.f;

			for (auto i = 0u; i < 3u; i++) {
				const auto change = trace.plane.normal[i] * back_off;

				velocity[i] = m_velocity[i] - change;

				if (std::fabsf(velocity[i]) >= 1.f)
					continue;

				velocity[i] = 0.f;
			}

			velocity *= std::clamp< float >(surface_elasticity * 0.45f, 0.f, 0.9f);

			if (trace.plane.normal.z > 0.7f) {
				const auto speed_sqr = velocity.LengthSqr();
				if (speed_sqr > 96000.f) {
					const auto l = velocity.Normalized().Dot(trace.plane.normal);
					if (l > 0.5f) {
						velocity *= 1.f - l + 0.5f;
					}
				}

				if (speed_sqr < 400.f) {
					m_velocity = {};
				}
				else {
					m_velocity = velocity;

					physics_push_entity(velocity * ((1.f - trace.fraction) * m_globals()->m_intervalpertick), trace);
				}
			}
			else {
				m_velocity = velocity;

				physics_push_entity(velocity * ((1.f - trace.fraction) * m_globals()->m_intervalpertick), trace);
			}

			if (m_bounces_count > 20)
				return detonate< false >();

			++m_bounces_count;
		}

		__forceinline void think() {
			switch (m_index) {
			case WEAPON_SMOKEGRENADE:
				if (m_velocity.LengthSqr() <= 0.01f) {
					detonate< false >();
				}

				break;
			case WEAPON_DECOY:
				if (m_velocity.LengthSqr() <= 0.04f) {
					detonate< false >();
				}

				break;
			case WEAPON_FLASHBANG:
			case WEAPON_HEGRENADE:
			case WEAPON_MOLOTOV:
			case WEAPON_INCGRENADE:
				if (TICKS_TO_TIME(m_tick) > m_detonate_time) {
					detonate< false >();
				}

				break;
			}

			m_next_think_tick = m_tick + TIME_TO_TICKS(0.2f);
		}

		template < bool _bounced >
		__forceinline void detonate() {
			m_detonated = true;

			update_path< _bounced >();
		}

		template < bool _bounced >
		__forceinline void update_path() {
			m_last_update_tick = m_tick;

			m_path.emplace_back(m_origin, _bounced);
		}
	public:
		__forceinline data_t() = default;

		_forceinline data_t(player_t* owner, int index, const Vector& origin, const Vector& velocity, float throw_time, int offset) : data_t() {
			m_owner = owner;
			m_index = index;

			predict(origin, velocity, throw_time, offset);
		}

		__forceinline void predict(const Vector& origin, const Vector& velocity, float throw_time, int offset) {
			m_origin = origin;
			m_velocity = velocity;
			m_collision_group = COLLISION_GROUP_PROJECTILE;

			const auto tick = TIME_TO_TICKS(1.f / 30.f);

			m_last_update_tick = -tick;

			switch (m_index) {
			case WEAPON_SMOKEGRENADE: m_next_think_tick = TIME_TO_TICKS(1.5f); break;
			case WEAPON_DECOY: m_next_think_tick = TIME_TO_TICKS(2.f); break;
			case WEAPON_FLASHBANG:
			case WEAPON_HEGRENADE:
				m_detonate_time = 1.5f;
				m_next_think_tick = TIME_TO_TICKS(0.02f);

				break;
			case WEAPON_INCGRENADE:
			case WEAPON_MOLOTOV:
				static const auto molotov_throw_detonate_time = m_cvar()->FindVar(crypt_str("molotov_throw_detonate_time"));

				m_detonate_time = molotov_throw_detonate_time->GetFloat();
				m_next_think_tick = TIME_TO_TICKS(0.02f);

				break;
			}

			for (auto i = 0; i < TIME_TO_TICKS(60.f); ++i) {
				m_tick = i;

				if (m_next_think_tick <= m_tick) {
					think();
				}

				if (i < offset)
					continue;

				if (physics_simulate())
					break;

				if (m_last_update_tick + tick > i)
					continue;

				update_path< false >();
			}

			if (m_last_update_tick + tick <= m_tick) {
				update_path< false >();
			}

			m_expire_time = throw_time + TICKS_TO_TIME(m_tick);
		}

		bool draw() const {
			if (m_path.size() <= 1u
				|| m_globals()->m_curtime > m_expire_time)
				return false;

			auto prev_nw2s_screen = ZERO;

			auto prev_screen = ZERO;
			auto pingzete_meduim_iq = std::get< Vector >(m_path.back()); // конечная точка куда прилетит нейд
			for (auto i = 1u; i < m_path.size(); ++i) {
				auto cur_screen = ZERO;

				std::vector<Vector>grenades;

				if (!math::world_to_screen(std::get< Vector >(m_path.at(i)), cur_screen))
					continue;

				switch (g_cfg.esp.grenadetracer)
				{
				case 1:
					if (!prev_screen.IsZero())
						//bullettracers::get().DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(g_cfg.esp.grenadetrace));
					//if (!g_cfg.warning.trace.visible_only)
						//bullettracers::get().DrawBeamPawWall(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(g_cfg.esp.grenadetrace));
						bullettracers::get().draw_grenade_beam(prev_nw2s_screen, std::get<Vector>(m_path.at(i)), Color(g_cfg.esp.grenadetrace));
					// его
					////DrawBeamPaw(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(rainbow_col));
					//if (!g_cfg.warning.trace.visible_only)
					//	DrawBeamPawWall(std::get< Vector >(m_path.at(i - 1)), std::get< Vector >(m_path.at(i)), Color(rainbow_col));
					break;
				case 2:
					if (!prev_screen.IsZero())

						render::get().line(prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, Color(g_cfg.esp.grenadetrace)); 
					break;
				}

				//if (g_cfg.esp.grenadetracer)
				//{

				//if (!prev_screen.IsZero())
				//	bullettracers::get().draw_grenade_beam(prev_nw2s_screen, std::get<Vector>(m_path.at(i)), Color(255, 255, 255, 255));
				//	//bullettracers::get().draw_grenade_beam(prev_screen, cur_screen, g_cfg.esp.grenadetrace);
				//}

				prev_nw2s_screen = std::get<Vector>(m_path.at(i));

				prev_screen = cur_screen;
			}

			bool safe = true;
			//auto factor1 = ;
			Vector nadeEnd = ZERO;

			Ray_t ray;
			Vector NadeScreen;
			math::world_to_screen(pingzete_meduim_iq, NadeScreen);

			// main hitbox, that takes damage
			auto player = g_ctx.local();
			Vector vPelvis = player->hitbox_position(HITBOX_PELVIS);
			ray.Init(pingzete_meduim_iq, vPelvis);
			trace_t ptr;

			CTraceFilter filter;

			m_trace()->TraceRay(ray, MASK_SHOT, &filter, &ptr);
			//trace to it

			if (ptr.hit_entity == player) {
				Vector PelvisScreen;

				math::world_to_screen(vPelvis, PelvisScreen);

				static float a = 105.0f;
				static float b = 25.0f;
				static float c = 140.0f;

				auto collideable = player->GetCollideable();

				auto origin = player->m_vecOrigin();
				auto min = collideable->OBBMins() + origin;
				auto max = collideable->OBBMaxs() + origin;

				auto center = min + (max - min) * 0.5f;

				// get delta between center of mass and final nade pos.
				auto delta = center - pingzete_meduim_iq;

				float d = ((delta.Length() - b) / c);
				float flDamage = a * exp(-d * d);

				auto dmg = max(static_cast<int>(ceilf(CSGO_Armor1(flDamage, player->m_ArmorValue()))), 0);
				dmg = min(dmg, (player->m_ArmorValue() > 0) ? 57 : 98);

				safe = dmg < 1;
			}
			auto dist_in_ft = [](Vector o, Vector dest)
			{
				Vector yo = Vector(dest.x - o.x, dest.y - o.y, dest.z - o.z);
				return std::roundf(std::sqrt(yo.x * yo.x + yo.y * yo.y + yo.z * yo.z));
			};
			auto eye_pos = g_ctx.local()->m_vecOrigin() + g_ctx.local()->m_vecViewOffset();
			auto dist = dist_in_ft(eye_pos, pingzete_meduim_iq);
			// пинг даун

			auto isOnScreen = [](Vector origin, Vector& screen) -> bool
			{
				if (!math::world_to_screen(origin, screen))
					return false;

				static int iScreenWidth, iScreenHeight;
				m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

				auto xOk = iScreenWidth > screen.x;
				auto yOk = iScreenHeight > screen.y;

				return xOk && yOk;
			};

			Vector screenPos;
			Vector vLocalOrigin = g_ctx.local()->GetAbsOrigin();

			if (!g_ctx.local()->is_alive())
				vLocalOrigin = m_input()->m_vecCameraOffset;

			auto rotate_svastik = [](Vector2D& point, Vector2D origin, bool clockwise, float angle) {
				Vector2D delta = point - origin;
				Vector2D rotated;

				if (clockwise) {
					rotated = Vector2D(delta.x * cosf(angle) - delta.y * sinf(angle), delta.x * sinf(angle) + delta.y * cosf(angle));
				}
				else {
					rotated = Vector2D(delta.x * sinf(angle) - delta.y * cosf(angle), delta.x * cosf(angle) + delta.y * sinf(angle));
				}

				point = rotated + origin;
			};

			if (!isOnScreen(pingzete_meduim_iq, screenPos))
			{
				int screen_size_x, screen_size_y;
				m_engine()->GetScreenSize(screen_size_x, screen_size_y);

				const float wm = screen_size_x / 2, hm = screen_size_y / 2;
				Vector last_pos = std::get< Vector >(m_path.at(m_path.size() - 1));

				Vector dir;

				m_engine()->GetViewAngles(dir);

				float view_angle = dir.y;

				if (view_angle < 0)
					view_angle += 360;

				view_angle = DEG2RAD(view_angle);

				auto entity_angle = math::calculate_angle(vLocalOrigin, pingzete_meduim_iq);
				entity_angle.Normalize();

				if (entity_angle.y < 0.f)
					entity_angle.y += 360.f;

				entity_angle.y = DEG2RAD(entity_angle.y);
				entity_angle.y -= view_angle;

				auto position = Vector(wm, hm, 0);
				position.x -= std::clamp(vLocalOrigin.DistTo(pingzete_meduim_iq), 100.f, hm - 100);

				auto new_pos = Vector2D(position.x, position.y);

				rotate_svastik(new_pos, Vector2D(wm, hm), false, entity_angle.y);

				nadeEnd = Vector(new_pos.x, new_pos.y, 0);
			}
			else
				math::world_to_screen(pingzete_meduim_iq, nadeEnd);

			if (!nadeEnd.IsZero()) {
				render::get().circle_filled(nadeEnd.x, nadeEnd.y, 60, 28, Color(0, 0, 0, 240));
				render::get().text(fonts[WARNING], nadeEnd.x - 4, nadeEnd.y - 22, Color(255, 250, 175, 255), FONTFLAG_DROPSHADOW, "!");
				render::get().text(fonts[WARNING1], nadeEnd.x - 12, nadeEnd.y + 10, Color(255, 255, 255, 255), FONTFLAG_DROPSHADOW, "%iFT", (int)dist);

				auto factor1 = (m_expire_time - m_globals()->m_curtime) / TICKS_TO_TIME(m_tick);
				otheresp::get().draw_arc(nadeEnd.x, nadeEnd.y, 26, 28, -90, 360 * factor1, Color(255, 255, 255, 255));
			}

			return true;
		}
	} m_data{};

	__forceinline c_grenade_prediction() = default;

	void on_create_move();

	__forceinline const data_t& get_local_data() const { return m_data; }
};