#pragma once

#include "Buffer.hpp"
#include "Creature.hpp"
#include "Egg.hpp"
#include "Food.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "macro.hpp"
#include <list>

class Simulation
{
	public:
		SimulationRules simulationRules;
		Environment		env;

		bool active;

		int frame = 0;

		int	  &foodCap				= simulationRules.foodCap;
		float &energyCostMultiplier = simulationRules.energyCostMultiplier;

        Simulation()
        {
            env.setupTraits<Creature>();
            env.setupTraits<Food>();
            env.setupTraits<Meat>();
            env.setupTraits<Egg>();
            env.setupTraits<TestObj>();
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
		// depricated - WILL REMOVE
		void removeFood(Food *food);

		void addMeat(agl::Vec<float, 2> position);
		void addMeat(agl::Vec<float, 2> position, float energy);
		void removeMeat(std::list<BaseEntity *>::iterator meat);
		// depricated - WILL REMOVE
		void removeMeat(Meat *meat);
};
