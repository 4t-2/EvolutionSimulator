#pragma once

#include "../lib/AGL/agl.hpp"
#include "../lib/PHY/fastphys.hpp"

class Food : public phy::Circle
{
	public:
		Food() : phy::Circle(*(agl::Circle *)0)
		{
			mass	 = 1;
			radius = 5;
		}

		int	  id;
		float energy;
};
