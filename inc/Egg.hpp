#pragma once

#include "Creature.hpp"

class Egg
{
	public:
		agl::Vec<float, 2> position;
		bool			   exists;
		CreatureData	   creatureData;
		int				   timeleft = 0;

		Egg()
		{
		}

		void setup(CreatureData &creatureData);
		void update();
		void clear();
};
