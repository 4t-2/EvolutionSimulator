#pragma once

#include "Creature.hpp"

class Egg
{
	private:
		int			  timeleft	   = 0;
		CreatureData *creatureData = nullptr;

	public:
		void setup(CreatureData *creatureData);
		void update();
		void clear();

		CreatureData *getCreatureData();
		int			  getTimeLeft();
};