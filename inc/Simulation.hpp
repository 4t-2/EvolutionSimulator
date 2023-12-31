#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Meat.hpp"
#include "PhysicsObj.hpp"
#include "SimulationRules.hpp"
#include "macro.hpp"
#include <list>

class Simulation
{
	public:
		SimulationRules simulationRules;
		Environment		env;

		b2World *phyWorld;

		agl::Vec<float, 2> gravity;

		bool active;

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

		Simulation()
		{
			env.setupTraits<PhyRect>();
		}

		void create(SimulationRules simulationRules, int seed);
		void destroy();

		void threadableUpdate();
		void updateSimulation();
		void update();

		static Buffer		creatureDataToBuffer(CreatureData &creatureData);
		static CreatureData bufferToCreatureData(Buffer buffer);

		void addCreature(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeCreature(std::list<BaseEntity *>::iterator creature);

		void addEgg(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeEgg(std::list<BaseEntity *>::iterator egg);

		void addFood(agl::Vec<float, 2> position);
		void removeFood(std::list<BaseEntity *>::iterator food);

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(std::list<BaseEntity *>::iterator meat);
};
