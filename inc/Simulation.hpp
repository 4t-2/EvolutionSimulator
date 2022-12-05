#pragma once

#include "Creature.hpp"
#include "Food.hpp"

class Simulation
{
	private:
		agl::Vec<float, 2> size;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;

		Food *food;

		int maxCreatures;
		int totalFood;

	public:
		Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood);
		void destroy();

		void updateCreatures();
		void updateFood();

		Creature *getCreatureBuffer();
		int		  getMaxCreatures();

		Food *getFood();
		int	  getTotalFood();

		List<Creature *> *getExistingCreatures();

		agl::Vec<float, 2> getSize();
};
