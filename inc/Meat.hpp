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
            size = {5, 5};
		}

		float lifetime	= 0;
		float rotation	= 0;
		float energyVol = 60;
};
