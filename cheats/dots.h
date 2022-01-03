#pragma once
#include "sdk/math/Vector.hpp"

/* pasted from aaron, thanks <3 */

class dot {
public:
	dot(Vector p, Vector v) {
		m_vel = v;
		m_pos = p;
	}

	void update();
	void draw();

	Vector m_pos, m_vel;
};

extern void dot_draw();
extern void dot_destroy();