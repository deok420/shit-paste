#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

enum Prediction_stage
{
	SETUP,
	PREDICT,
	FINISH
};

class engineprediction : public singleton <engineprediction>
{
	struct Netvars_data
	{
		int tickbase = INT_MIN;

		int m_flags = 0;

		int m_move_type = 0;

		CUserCmd* cmd;

		Vector m_aimPunchAngle = ZERO;
		Vector m_origin = ZERO;
		Vector m_velocity = ZERO;
		Vector m_aimPunchAngleVel = ZERO;
		Vector m_viewPunchAngle = ZERO;
		Vector m_vecViewOffset = ZERO;

		float m_duckAmount = 0.f;
		float m_duckSpeed = 0.f;
		float m_recoil_index = 0.f;
		float m_accuracy_penalty = 0.f;
		float m_surface_friction = 0.f;
		float m_fallVelocity = 0.f;
		float m_velocityModifier = 0.f;
	};

	struct Backup_data
	{
		int flags = 0;
		Vector velocity = ZERO;
	};

	struct Prediction_data
	{
		void reset()
		{
			prediction_stage = SETUP;
			old_curtime = 0.0f;
			old_frametime = 0.0f;
		}

		Prediction_stage prediction_stage = SETUP;
		float old_curtime = 0.0f;
		float old_frametime = 0.0f;
		int* prediction_random_seed = nullptr;
		int* prediction_player = nullptr;
	};

	struct Viewmodel_data
	{
		weapon_t* weapon = nullptr;

		int viewmodel_index = 0;
		int sequence = 0;
		int animation_parity = 0;

		float cycle = 0.0f;
		float animation_time = 0.0f;
	};
public:
	Netvars_data netvars_data[MULTIPLAYER_BACKUP];
	Backup_data backup_data;
	Prediction_data prediction_data;
	Viewmodel_data viewmodel_data;

	void store_netvars();
	void restore_netvars();
	void setup();
	void predict(CUserCmd* m_pcmd);
	void finish();
};