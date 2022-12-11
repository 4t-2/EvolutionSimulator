#pragma once

#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"

class Simulation
{
	private:
		agl::Vec<float, 2> size;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;

		Egg *eggBuffer;
		List<Egg*> *existingEggs;

		int currentId = 1;

		Food *food;

		int maxEggs;
		int maxCreatures;
		int totalFood;

	public:
		Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood, int maxEggs);
		void destroy();

		void update();

		static Buffer		*creatureDataToBuffer(CreatureData *creatureData);
		static CreatureData *bufferToCreatureData(Buffer buffer);
		static void			 mutateBuffer(Buffer *buffer, int chance);

		void addCreature(CreatureData *creatureData);
		void killCreature(Creature *creature);

		void addEgg(CreatureData*);

		int		  getMaxCreatures();
		Creature *getCreatureBuffer();
		List<Creature *> *getExistingCreatures();

		int getMaxEggs();
		Egg* getEggBuffer();
		List<Egg *> *getExistingEggs();

		int	  getTotalFood();
		Food *getFood();

		agl::Vec<float, 2> getSize();
};
