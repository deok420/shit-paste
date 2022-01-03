#pragma once

#include "..\..\includes.hpp"

class circlestrafer : public singleton< circlestrafer > {
public:
	void start( );
	void strafe( );

	void circle_strafe(CUserCmd * cmd, float * circle_yaw);

	bool active;
	float right_movement;
	float strafe_angle;
};