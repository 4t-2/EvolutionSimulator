#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "macro.hpp"
#include <list>
#include "EnDex.hpp"

class Simulation
{
	public:
		SimulationRules simulationRules;
		EnDex<ENVTYPES>*		env;

		bool active;

		int frame = 0;

		int	  &foodCap				= simulationRules.foodCap;
		float &energyCostMultiplier = simulationRules.energyCostMultiplier;

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

		void addCreature(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeCreature(std::list<Creature>::iterator creature);

		void addEgg(CreatureData &creatureData, agl::Vec<float, 2> position);
		void removeEgg(std::list<Egg>::iterator egg);

		void addFood(agl::Vec<float, 2> position);
		void removeFood(std::list<Food>::iterator food);

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(std::list<Meat>::iterator meat);
};
