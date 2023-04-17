#pragma once

#include "../lib/AGL/agl.hpp"
#include "../lib/PHY/fastphys.hpp"
#include <cstdlib>
#include "macro.hpp"

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
		bool exists = true;

#ifdef ACTIVEFOOD
		agl::Vec<float, 2> nextPos;

		void nextRandPos(agl::Vec<float, 2> worldSize)
		{
			constexpr float range = ACTIVEFOOD;

			float xOffset = rand() / (float)RAND_MAX;
			xOffset -= .5;
			xOffset *= 2 * range;

			float yOffset = rand() / (float)RAND_MAX;
			yOffset -= .5;
			yOffset *= 2 * range;

			nextPos.x = std::max((float)0, std::min(worldSize.x, position.x + xOffset));
			nextPos.y = std::max((float)0, std::min(worldSize.y, position.y + yOffset));
		}
#endif
};
