#pragma once

#include "../lib/AGL/agl.hpp"
#include "CreatureData.hpp"
#include "Food.hpp"
#include "Grid.hpp"
#include "Meat.hpp"
#include "SimulationRules.hpp"
#include "other.hpp"
#include "macro.hpp"

#define sizeToHealth(size) 100*size*size*size

class Creature : public phy::Circle
{
	public:
		float rotation = 0;

		agl::Vec<int, 2> gridPosition = {0, 0};

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

		bool existing = false;

		SimulationRules *simulationRules;

		CreatureData creatureData;

		agl::Vec<int, 2> startGridOffset;
		agl::Vec<int, 2> endGridOffset;

		Creature();
		~Creature();

		void setup(CreatureData &creatureData, SimulationRules *simulationRules);
		void clear();

		void updateNetwork(Grid<Food *> *foodGrid, Grid<Creature *> *creatureGrid, Grid<Meat *> *meatGrid);
		void updateActions(float energyCostMultiplier);
};
