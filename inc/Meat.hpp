#pragma once

#include "Food.hpp"
#include <AGL/agl.hpp>

class Meat : public PhysicsObj
{
	public:
		Meat()
		{
			setup({0, 0}, {10, 10}, 1);
		}

		float lifetime	= 0;
		float energyVol = 60;
};
