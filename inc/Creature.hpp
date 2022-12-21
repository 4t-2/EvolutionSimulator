#pragma once

#include "../lib/AGL/agl.hpp"
#include "CreatureData.hpp"
#include "Food.hpp"
#include "List.hpp"
#include "NeuralNetwork.hpp"
#include "other.hpp"

#define RAY_LENGTH 1000

#define TOTAL_INPUT	 (10)
#define TOTAL_OUTPUT 6
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_OUTPUT)

#define CONSTANT_INPUT	  0
#define X_INPUT			  1
#define Y_INPUT			  2
#define ROTATION_INPUT	  3
#define SPEED_INPUT		  4
#define FOOD_DISTANCE	  6
#define FOOD_ROTATION	  7
#define CREATURE_DISTANCE 8
#define CREATURE_ROTATION 9

#define FOWARD_OUTPUT	(TOTAL_INPUT + 0)
#define BACKWARD_OUTPUT (TOTAL_INPUT + 1)
#define RIGHT_OUTPUT	(TOTAL_INPUT + 2)
#define LEFT_OUTPUT		(TOTAL_INPUT + 3)
#define EAT_OUTPUT		(TOTAL_INPUT + 4)
#define LAYEGG_OUTPUT	(TOTAL_INPUT + 5)

class Creature
{
	private:
		agl::Vec<float, 2> position		= {0, 0};
		agl::Vec<float, 2> velocity		= {0, 0};
		agl::Vec<float, 2> acceleration = {0, 0};
		float			   rotation		= 0;

		float radius = 0;

		NeuralNetwork *network = nullptr;

		bool eating	   = false;
		bool layingEgg = false;

		float sight = 0;
		float speed = 0;
		float tough = 0;

		// sight + (speed^2)(tough^2)
		float energy = 0;
		float health = 0;

		int lifeLeft = 0;

		CreatureData creatureData;

	public:
		float closest;
		float closestAngle;

		Creature();
		~Creature();

		void setup(CreatureData &creatureData);
		void clear();

		void setPosition(agl::Vec<float, 2> position);
		void setVelocity(agl::Vec<float, 2> velocity);
		void setRotation(float rotation);
		void setHealth(float health);
		void setEnergy(float energy);

		void updateNetwork(List<Food *> *existingFood, List<Creature *> *existingCreatures,
						   agl::Vec<float, 2> worldSize);
		void updateActions(Food *food);

		CreatureData getCreatureData();

		NeuralNetwork	   getNeuralNetwork();
		agl::Vec<float, 2> getPosition();
		agl::Vec<float, 2> getVelocity();
		float			   getRotation();
		bool			   getEating();
		bool			   getLayingEgg();
		float			   getHealth();
		float			   getEnergy();
		int				   getLifeLeft();
};
