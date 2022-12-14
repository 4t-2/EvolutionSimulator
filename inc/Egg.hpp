#pragma once

#include "Creature.hpp"

class Egg
{
	private:
		agl::Vec<float, 2> position;
		CreatureData	   creatureData;
		int				   timeleft = 0;

	public:
		void setup(CreatureData &creatureData);
		void update();
		void clear();

		void setPosition(agl::Vec<float, 2> position);

		agl::Vec<float, 2> getPosition();
		CreatureData	  &getCreatureData();
		int				   getTimeLeft();
};
