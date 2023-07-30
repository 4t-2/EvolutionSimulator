#pragma once

#include <AGL/agl.hpp>
#include "Food.hpp"

class Meat : public phy::Circle
{
	public:
		Meat() : phy::Circle(*(agl::Circle *)0)
		{
			mass = 2;
			radius = 5;
		}

		float lifetime = 0;
		float rotation = 0;
		float energyVol = 60;
		bool exists;
};
