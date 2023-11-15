#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Grid.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "macro.hpp"
#include <list>

class Simulation
{
	public:
		SimulationRules simulationRules;

		bool active;

		std::list<Creature> existingCreatures;
		std::list<Egg>		existingEggs;
		std::list<Food>		existingFood;
		std::list<Meat>		existingMeat;

		Grid<Creature *> *creatureGrid;
		Grid<Food *>	 *foodGrid;
		Grid<Meat *>	 *meatGrid;

		std::vector<int> totalNEAT;
		std::vector<int> totalRL;
		std::vector<int> totalBOTH;

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
