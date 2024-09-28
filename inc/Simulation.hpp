#pragma once

#include "Buffer.hpp"
#include "EnDex.hpp"
#include "Food.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "ViteSeg.hpp"
#include "macro.hpp"
#include <list>

class Simulation
{
	public:
		SimulationRules	 simulationRules;
		EnDex<ENVTYPES> *env;

		bool active;

		int frame = 0;

		int	  &foodCap				= simulationRules.foodCap;
		float &energyCostMultiplier = simulationRules.energyCostMultiplier;

		std::vector<ViteGenome> startingGenome = {
			ViteGenome{{24, 24}, 0, 0, 0, 100}, ViteGenome{{10, 18}, 0, 0, 0, 100}, ViteGenome{{4, 18}, 0, 0, 0, 100}};

		Simulation()
		{
		}

		void create(SimulationRules simulationRules, int seed);
		void destroy();

		void threadableUpdate();
		void updateSimulation();
		void update();

		static Buffer		creatureDataToBuffer(CreatureData &creatureData);
		static CreatureData bufferToCreatureData(Buffer buffer);

		void addCreature(std::vector<ViteGenome> genome, agl::Vec<float, 2> position);
		void removeCreature(std::list<ViteSeg>::iterator viteSeg);

		void addFood(agl::Vec<float, 2> position);
		void removeFood(std::list<Food>::iterator food);

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(std::list<Meat>::iterator meat);
};
