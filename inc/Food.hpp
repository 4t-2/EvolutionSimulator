#pragma once

#include "../lib/AGL/agl.hpp"
#include "../lib/PHY/fastphys.hpp"

class Food : public phy::Circle
{
	public:
		Food() : phy::Circle(*(agl::Circle *)0)
		{
		}

		float mass	 = 1;
		float radius = 5;

		int	  id;
		float energy;
};
