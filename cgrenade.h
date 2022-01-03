#pragma once
#include <Windows.h>
#include <vector>

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

struct Grenade_t
{
	entity_t* entity;
	std::vector<Vector> positions;
	float addTime;
};

class CGrenade
{
public:
	CGrenade();

	bool checkGrenades(entity_t* ent);
	void addGrenade(Grenade_t grenade);
	void updatePosition(entity_t* ent, Vector position);
	void draw();

private:

	std::vector<Grenade_t> grenades;
	int findGrenade(entity_t* ent);

};