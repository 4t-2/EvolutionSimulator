#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Grid.hpp"
#include "SimulationRules.hpp"
#include "Meat.hpp"

#define MAXMEAT 2000

class Simulation
{
	public:
		SimulationRules simulationRules;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;
		Grid<Creature *> *creatureGrid;

		Egg			*eggBuffer;
		List<Egg *> *existingEggs;

		Food		 *foodBuffer;
		List<Food *> *existingFood;
		Grid<Food *> *foodGrid;

		std::vector<int> creaturePopData;
		std::vector<float> creatureSightData;
		std::vector<float> creatureSpeedData;
		std::vector<float> creatureSizeData;

		Meat *meatBuffer;
		List<Meat *> *existingMeat;
		Grid<Meat*> *meatGrid;

		int frame =0 ;

		Simulation(SimulationRules simulationRules);
		void destroy();

		void threadableUpdate();
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

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(Meat *meat);
};
