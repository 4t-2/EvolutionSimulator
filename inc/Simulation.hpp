#pragma once

#include "Creature.hpp"
#include "Food.hpp"

class Simulation
{
	private:
		agl::Vec<float, 2> size;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;

		int currentId = 1;

		Food *food;

		int maxCreatures;
		int totalFood;

	public:
		Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood);
		void destroy();

		void updateCreatures();
		void updateFood();

		static Buffer		*creatureDataToBuffer(CreatureData *creatureData);
		static CreatureData *bufferToCreatureData(Buffer buffer);
		static void			 mutateBuffer(Buffer *buffer, int chance);

		void addCreature(CreatureData *creatureData);
		void killCreature(Creature *creature);

		Creature *getCreatureBuffer();
		int		  getMaxCreatures();

		Food *getFood();
		int	  getTotalFood();

		List<Creature *> *getExistingCreatures();

		agl::Vec<float, 2> getSize();
};
