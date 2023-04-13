#pragma once

#include "../lib/AGL/agl.hpp"
#include "CreatureData.hpp"
#include "Food.hpp"
#include "Grid.hpp"
#include "Meat.hpp"
#include "NeuralNetwork.hpp"
#include "SimulationRules.hpp"
#include "other.hpp"

#define RAY_LENGTH 1000

#define TOTAL_INPUT	 15
#define TOTAL_OUTPUT 5
#define TOTAL_HIDDEN 4
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_OUTPUT + TOTAL_HIDDEN)

#define CONSTANT_INPUT		0
#define X_INPUT				1
#define Y_INPUT				2
#define ROTATION_INPUT		3
#define SPEED_INPUT			4
#define FOOD_DISTANCE		5
#define FOOD_ROTATION		6
#define CREATURE_DISTANCE	7
#define CREATURE_ROTATION	8
#define ENERGY_INPUT		9
#define HEALTH_INPUT		10
#define LIFE_INPUT			11
#define MEAT_DISTANCE		12
#define MEAT_ROTATION		13
#define CREATURE_PREFERENCE 14

#define FOWARD_OUTPUT (TOTAL_INPUT + 0)
#define RIGHT_OUTPUT  (TOTAL_INPUT + 1)
#define LEFT_OUTPUT	  (TOTAL_INPUT + 2)
#define EAT_OUTPUT	  (TOTAL_INPUT + 3)
#define LAYEGG_OUTPUT (TOTAL_INPUT + 4)

class Creature : public phy::Circle
{
	public:
		float rotation = 0;

		agl::Vec<int, 2> gridPosition = {0, 0};

		NeuralNetwork *network = nullptr;

		bool eating	   = false;
		bool layingEgg = false;

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

		float &preference = creatureData.preference; // plant = 1
													 // meat  = 0

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
		void updateActions();
};
