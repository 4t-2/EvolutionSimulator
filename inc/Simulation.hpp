#pragma once

#include "Creature.hpp"
#include "Food.hpp"

class Simulation
{
	private:
		agl::Vec<float, 2> size;

		Creature *creature;
		Food	 *food;

		int totalCreatures;
		int totalFood;

	public:
		Simulation(agl::Vec<float, 2>size, int totalCreatures, int totalFood);
		void destroy();

		void updateCreatures();
		void updateFood();

		Creature *getCreature();
		Food *getFood();

		int getTotalFood();
		int getTotalCreatures();
};
