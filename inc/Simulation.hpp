#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Grid.hpp"

class Simulation
{
	private:
		agl::Vec<float, 2> size;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;
		int				  maxCreatures;

		Egg			*eggBuffer;
		List<Egg *> *existingEggs;
		int			 maxEggs;

		Food		 *foodBuffer;
		List<Food *> *existingFood;
		int			  maxFood;

		Grid<Food *> *foodGrid;
		Grid<Creature *> *creatureGrid;

	public:
		Simulation(agl::Vec<float, 2> size, int maxCreatures, int maxFood, int maxEggs);
		void destroy();

		void updateNetworks();
		void updateSimulation();
		void update();

		static Buffer		creatureDataToBuffer(CreatureData &creatureData);
		static CreatureData bufferToCreatureData(Buffer buffer);
		static void			mutateBuffer(Buffer *buffer, int chance);

		void addCreature(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeCreature(Creature *creature);

		void addEgg(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeEgg(Egg *egg);

		void addFood(agl::Vec<float, 2> position);
		void removeFood(Food *food);

		Creature		 *getCreatureBuffer();
		List<Creature *> *getExistingCreatures();
		int				  getMaxCreatures();

		Egg			*getEggBuffer();
		List<Egg *> *getExistingEggs();
		int			 getMaxEggs();

		Food		 *getFoodBuffer();
		List<Food *> *getExistingFood();
		int			  getMaxFood();

		agl::Vec<float, 2> getSize();
};
