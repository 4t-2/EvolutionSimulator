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
			setup({0, 0}, {10, 10}, 0);
		}

		float lifetime	= 0;
		float energyVol = 60;
};
