#pragma once

#include "Food.hpp"
#include <AGL/agl.hpp>

class Meat : public Entity<PhysicsObj>
{
	public:
        bool exists;
        agl::Vec<float, 2> position;

		Meat() : Entity<PhysicsObj>(exists, position)
		{
			mass   = 2;
			// radius = 5;
            size = {10, 10};
		}

		float lifetime	= 0;
		float energyVol = 60;
};
