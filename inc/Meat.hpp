#pragma once

#include "Food.hpp"
#include <AGL/agl.hpp>

class Meat : public Entity<PhysicsObj>
{
	public:
		Meat()
		{
			mass   = 2;
			radius = 5;
		}

		float lifetime	= 0;
		float rotation	= 0;
		float energyVol = 60;
};
