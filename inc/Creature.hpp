#pragma once

#include "NeuralNetwork.hpp"
#include "../lib/AGL/agl.hpp"

class Creature
{
	private:
		agl::Vec2f position = {0, 0};
		agl::Vec2f velocity = {0, 0};
		float	   rotation = 0;

		agl::Vec2f worldSize = {0, 0};

		// first 2 neurons are input
		// next 3 are output
		// rest are hidden
		NeuralNetwork *network;

	public:
		Creature();
		
		void setPosition(agl::Vec2f position);
		void setWorldSize(agl::Vec2f worldSize);

		void update();

		NeuralNetwork getNeuralNetwork();
		agl::Vec2f	  getPosition();
		float		  getRotation();
};
