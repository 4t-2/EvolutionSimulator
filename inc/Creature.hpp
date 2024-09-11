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
	std::vector<float> state;
	std::vector<float> action;
	float reward;
};

struct RelPos
{
	float rotation = 0;
	float distance = 0;
};

class Creature : public PhysicsObj
{
	public:
        bool exists;
        agl::Vec<float, 2> position;

		in::NeuralNetwork *network = nullptr;

		bool eating	   = false;
		bool layingEgg = false;

		bool incubating = 0;
		
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

		CreatureData creatureData;

		std::vector<PhysicsObj*> segments;

		Creature();
		~Creature();

		void clear();

		void learnBrain(SimulationRules &simRules);
		void updateNetwork();
		void updateActions();
};
