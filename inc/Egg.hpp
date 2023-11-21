#pragma once

#include "Creature.hpp"

class Egg : public Entity<PhysicsObj>
{
	public:
		agl::Vec<float, 2> position;
		CreatureData	   creatureData;
		int				   timeleft = 0;

		void setup(CreatureData &creatureData);
		void update();
		void clear();
};
