#include "cgrenade.h"
#include "cheats/visuals/bullet_tracers.h"

CGrenade::CGrenade()
{

}

bool CGrenade::checkGrenades(entity_t* ent) //to check if we already added this grenade
{
	for (Grenade_t grenade : grenades)
		if (grenade.entity == ent)
			return false;

	return true;
}

void CGrenade::addGrenade(Grenade_t grenade)
{
	grenades.push_back(grenade);
}

void CGrenade::updatePosition(entity_t* ent, Vector position)
{
	grenades.at(findGrenade(ent)).positions.push_back(position);
}

void CGrenade::draw()
{
	for (size_t i = 0; i < grenades.size(); i++)
	{
		//if (grenades.at(i).addTime + 20.f < m_globals()->m_realtime)
		//	continue; // removing the grenade_t was causing crashes

		if (grenades.at(i).addTime + 2.5f < m_globals()->m_realtime)
		{
			if (grenades.at(i).positions.size() < 1) continue;

			grenades.at(i).positions.erase(grenades.at(i).positions.begin());
		}

		for (size_t j = 1; j < grenades.at(i).positions.size(); j++)
		{
			/*	Vector sPosition;
				Vector sLastPosition;
				Vector ToDo = grenades.at(i).positions.at(j);
				if (math::world_to_screen(ToDo, sPosition) && math::world_to_screen(grenades.at(i).positions.at(j - 1), sLastPosition))
					render::get().line(sPosition.x, sPosition.y, sLastPosition.x, sLastPosition.y, Color(255, 255, 0, 255));*/
			bullettracers::get().draw_grenade_beam(grenades.at(i).positions.at(j), grenades.at(i).positions.at(j - 1), Color(g_cfg.esp.grenadetrace));
		}
	}
}

int CGrenade::findGrenade(entity_t* ent)
{
	for (size_t i = 0; i < grenades.size(); i++)
		if (grenades.at(i).entity == ent)
			return i;

	return 0;
}