#pragma once

#include "../lib/AGL/agl.hpp"
#include "Food.hpp"
#include "NeuralNetwork.hpp"

#define RAY_TOTAL  10
#define RAY_LENGTH 500

#define TOTAL_INPUT	 (5 + (RAY_TOTAL * 2))
#define TOTAL_HIDDEN 6
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_HIDDEN)

#define TOTAL_CONNECTIONS 2

class Creature
{
	private:
		agl::Vec<float, 2> position = {0, 0};
		agl::Vec<float, 2> velocity = {0, 0};
		float			   rotation = 0;

		float radius;

		agl::Vec<float, 2> worldSize = {0, 0};

		NeuralNetwork *network;

		bool eating	   = false;
		bool layingEgg = false;

	public:
		float closest;
		float closestAngle;

		Creature();
		Creature(unsigned char data[TOTAL_CONNECTIONS * 3]);

		void setPosition(agl::Vec<float, 2> position);
		void setVelocity(agl::Vec<float, 2> velocity);
		void setRotation(float rotation);
		void setWorldSize(agl::Vec<float, 2> worldSize);

		void updateNetwork(Food *food, int totalFood, Creature* creature, int totalCreatures);
		void updateActions(Food *food);

		void saveData(unsigned char buffer[TOTAL_CONNECTIONS * 3]);

		static void mutateData(unsigned char buffer[TOTAL_CONNECTIONS * 3], int chance);

		NeuralNetwork	   getNeuralNetwork();
		agl::Vec<float, 2> getPosition();
		agl::Vec<float, 2> getVelocity();
		float			   getRotation();
		bool			   getEating();
		bool			   getLayingEgg();
};
