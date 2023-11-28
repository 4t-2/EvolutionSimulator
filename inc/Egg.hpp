#pragma once

#include "Creature.hpp"

class Egg : public Entity<>
{
	public:
		agl::Vec<float, 2> position;
		bool			   exists;
		CreatureData	   creatureData;
		int				   timeleft = 0;

		Egg() : Entity<>(exists, position)
		{
		}

		void setup(CreatureData &creatureData);
		void update();
		void clear();
};
