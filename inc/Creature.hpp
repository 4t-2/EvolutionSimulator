#pragma once

#include <AGL/agl.hpp>
#include "CreatureData.hpp"
#include "Food.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "other.hpp"
#include "macro.hpp"
#include "Environment.hpp"
#include "PhysicsObj.hpp"

#define sizeToHealth(size) 100*size*size*size

struct Memory
{
	float state[TOTAL_INPUT];
	float action[TOTAL_OUTPUT];
	float reward;
};

struct RelPos
{
	float rotation = 0;
	float distance = 0;
};

class Creature : public Entity<PhysicsObj>
{
	public:
        bool exists;
        agl::Vec<float, 2> position;
		float rotation = 0;

		RelPos creatureRelPos;
		RelPos foodRelPos;
		RelPos meatRelPos;

		in::NeuralNetwork *network = nullptr;

		bool eating	   = false;
		bool layingEgg = false;

		bool incubating = 0;
		
		float sight = 0;
		float speed = 0;
		float size	= 0;

		// sight + (speed^2)(size^3)
		float energy	= 0;
		float health	= 0;
		int	  life		= 0;
		float maxEnergy = 0;
		float maxHealth = 0;
		int	  maxLife	= 0;

		float maxBiomass	= 0;
		float biomass		= 0;
		float energyDensity = 0;

		float eggTotalCost	  = 0;
		float eggHealthCost = 0;
		float eggEnergyCost = 0;
		float eggDesposit = 0;

		float &preference = creatureData.preference; // plant = 1
													 // meat  = 0

		float &metabolism = creatureData.metabolism;

		float maxForce	  = 0;
		float maxRotation = 0;

		float rayLength = 0;

		int hue = 0;

		SimulationRules *simulationRules;

		CreatureData creatureData;

		agl::Vec<int, 2> startGridOffset;
		agl::Vec<int, 2> endGridOffset;

		Memory memory[240];
		float baseline = 0;
		float reward = 0;
		float shift[TOTAL_OUTPUT];

		Creature();
		~Creature();

		void setup(CreatureData &creatureData, SimulationRules *simulationRules);
		void clear();

		void learnBrain(SimulationRules &simRules);
		void updateNetwork();
		void updateActions();
};
