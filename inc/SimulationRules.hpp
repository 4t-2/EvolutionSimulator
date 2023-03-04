#pragma once

#include "../lib/AGL/agl.hpp"

class SimulationRules
{
	public:
		agl::Vec<int, 2> size;
		agl::Vec<int, 2> gridResolution;
		int				 startingCreatures;
		int				 foodEnergy;
		int				 maxCreatures;
		int				 maxFood;
		int				 maxEggs;
		int				 preferedCreatures;
		int				 penaltyBuffer;
		int				 penaltyPeriod;
};
