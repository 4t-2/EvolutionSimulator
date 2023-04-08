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

#define TOTAL_INPUT	 14
#define TOTAL_OUTPUT 5
#define TOTAL_HIDDEN 4
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_OUTPUT + TOTAL_HIDDEN)

#define CONSTANT_INPUT	  0
#define X_INPUT			  1
#define Y_INPUT			  2
#define ROTATION_INPUT	  3
#define SPEED_INPUT		  4
#define FOOD_DISTANCE	  5
#define FOOD_ROTATION	  6
#define CREATURE_DISTANCE 7
#define CREATURE_ROTATION 8
#define ENERGY_INPUT	  9
#define HEALTH_INPUT	  10
#define LIFE_INPUT		  11
#define MEAT_DISTANCE 12
#define MEAT_ROTATION 13

#define FOWARD_OUTPUT (TOTAL_INPUT + 0)
#define RIGHT_OUTPUT  (TOTAL_INPUT + 1)
#define LEFT_OUTPUT	  (TOTAL_INPUT + 2)
#define EAT_OUTPUT	  (TOTAL_INPUT + 3)
#define LAYEGG_OUTPUT (TOTAL_INPUT + 4)

class Creature : public phy::Circle
{
	private:
		float			   rotation		= 0;

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

		float maxForce	  = 0;
		float maxRotation = 0;

		float rayLength = 0;

		int hue = 0;

		bool existing = false;

		SimulationRules *simulationRules;

		CreatureData creatureData;

	public:
		agl::Vec<int, 2> startGridOffset;
		agl::Vec<int, 2> endGridOffset;

		Creature();
		~Creature();

		void setup(CreatureData &creatureData, SimulationRules *simulationRules);
		void clear();

		void setPosition(agl::Vec<float, 2> position);
		void setVelocity(agl::Vec<float, 2> velocity);
		void setRotation(float rotation);
		void setHealth(float health);
		void setEnergy(float energy);
		void setGridPosition(agl::Vec<int, 2> gridPosition);

		void updateNetwork(Grid<Food *> *foodGrid, Grid<Creature *> *creatureGrid, Grid<Meat *> *meatGrid);
		void updateActions();

		CreatureData getCreatureData();

		NeuralNetwork	   getNeuralNetwork();
		float			   getRotation();
		bool			   getEating();
		bool			   getLayingEgg();
		float			   getHealth();
		float			   getEnergy();
		int				   getLifeLeft();
		float			   getSight();
		float			   getSpeed();
		float			   getSize();
		int				   getHue();
		float			   getRadius();
		agl::Vec<int, 2>   getGridPosition();
		float			   getMaxEnergy();
		float			   getMaxHealth();
		int				   getMaxLife();
};
