#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Grid.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "macro.hpp"

class Simulation
{
	public:
		SimulationRules simulationRules;

		bool active;

		Creature		 *creatureBuffer;
		List<Creature *> *existingCreatures;
		Grid<Creature *> *creatureGrid;

		Egg			*eggBuffer;
		List<Egg *> *existingEggs;

		Food		 *foodBuffer;
		List<Food *> *existingFood;
		Grid<Food *> *foodGrid;

		// std::vector<int>   creaturePopData;
		// std::vector<float> creatureSightData;
		// std::vector<float> creatureSpeedData;
		// std::vector<float> creatureSizeData;

		Meat		 *meatBuffer;
		List<Meat *> *existingMeat;
		Grid<Meat *> *meatGrid;

		int frame = 0;

		float &foodEnergyDensity	= simulationRules.foodEnergyDensity;
		float &meatEnergyDensity	= simulationRules.meatEnergyDensity;
		int	  &foodCap				= simulationRules.foodCap;
		float &foodVol				= simulationRules.foodVol;
		float &leachVol				= simulationRules.leachVol;
		float &damage				= simulationRules.damage;
		float &energyCostMultiplier = simulationRules.energyCostMultiplier;

		void create(SimulationRules simulationRules, int seed);
		void destroy();

		void threadableUpdate();
		void updateSimulation();
		void update();

		static Buffer		creatureDataToBuffer(CreatureData &creatureData);
		static CreatureData bufferToCreatureData(Buffer buffer);
		static void			mutateBuffer(Buffer *buffer, int chance);

		Creature *addCreature(CreatureData &creatureData, agl::Vec<float, 2> position);
		void	  removeCreature(Creature *creature);

		void addEgg(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeEgg(Egg *egg);

		void addFood(agl::Vec<float, 2> position);
		void removeFood(Food *food);

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(Meat *meat);
};
