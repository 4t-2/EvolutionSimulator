#pragma once

#include "../lib/AGL/agl.hpp"
#include "macro.hpp"

class SimulationRules
{
	public:
		// starting
		agl::Vec<int, 2> size;
		agl::Vec<int, 2> gridResolution;
		int				 startingCreatures;
		int				 foodEnergy;
		int				 maxCreatures;
		int				 maxFood;
		int				 maxEggs;

		// variable
		// int preferedCreatures;
		// int penaltyBuffer;
		// int penaltyPeriod;
		
		float foodEnergyDensity = FOODENERGY;
		float meatEnergyDensity = MEATENERGY;
		int foodCap;
		float foodVol  = FOODVOL;
		float leachVol = LEACHVOL;
		float damage = DAMAGE;
		float energyCostMultiplier = ENERGYCOSTMULTIPLIER;
};
