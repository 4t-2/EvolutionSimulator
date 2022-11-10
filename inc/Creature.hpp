#pragma once

#include "../lib/AGL/agl.hpp"
#include "Food.hpp"
#include "NeuralNetwork.hpp"

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

	public:
		Creature();

		void setPosition(agl::Vec<float, 2> position);
		void setWorldSize(agl::Vec<float, 2> worldSize);

		void updateActions(Food *food, int totalFood);

		NeuralNetwork getNeuralNetwork();
		agl::Vec<float, 2>	  getPosition();
		float		  getRotation();
		Food		 *getClosestFood();
};
