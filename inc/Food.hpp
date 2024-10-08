#pragma once

#include "Environment.hpp"
#include "PhysicsObj.hpp"
#include "macro.hpp"
#include <AGL/agl.hpp>
#include <cstdlib>

class Food : public PhysicsObj
{
	public:
		Food()
		{
            setup({0, 0}, {10, 10}, 1);
		}

		int	  id;
		float energy;

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
