#pragma once

#include "../lib/AGL/agl.hpp"
#include "Food.hpp"
#include "NeuralNetwork.hpp"

#define RAY_TOTAL 10
#define RAY_LENGTH 500

#define TOTAL_NODES 17
#define TOTAL_INPUT 11
#define TOTAL_CONNECTIONS 3

class Creature
{
	private:
		agl::Vec<float, 2> position = {0, 0};
		agl::Vec<float, 2> velocity = {0, 0};
		float	   rotation = 0;

		agl::Vec<float, 2> worldSize = {0, 0};

		// first 2 neurons are input
		// next 3 are output
		// rest are hidden
		NeuralNetwork *network;

		Food *closestFood;
		float foodDistance;

	public:
		Creature();

		void setPosition(agl::Vec<float, 2> position);
		void setWorldSize(agl::Vec<float, 2> worldSize);

		void updateNetwork(Food *food, int totalFood);
		void updateActions(Food *food);

		NeuralNetwork getNeuralNetwork();
		agl::Vec<float, 2>	  getPosition();
		float		  getRotation();
		Food		 *getClosestFood();
};
