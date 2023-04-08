#pragma once

#include "../lib/AGL/agl.hpp"
#include "Food.hpp"

class Meat : public phy::Circle
{
	public:
		Meat() : phy::Circle(*(agl::Circle *)0)
		{
			mass = 2;
			radius = 5;
		}

		float energy = 60;
		bool exists;
};
