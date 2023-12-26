#pragma once

#include "macro.hpp"
#include <AGL/agl.hpp>

class SimulationRules
{
	public:
		// starting
		agl::Vec<int, 2> size;
		agl::Vec<int, 2> gridResolution;
		int				 startingCreatures;
		int				 foodEnergy;

		// variable
		// int preferedCreatures;
		// int penaltyBuffer;
		// int penaltyPeriod;

		float foodEnergyDensity = FOODENERGY;
		float meatEnergyDensity = MEATENERGY;
		int	  foodCap;
		float foodVol			   = FOODVOL;
		float leachVol			   = LEACHVOL;
		float damage			   = DAMAGE;
		float energyCostMultiplier = ENERGYCOSTMULTIPLIER;

		int threads = THREADS;

		float learningRate	= .1;
		int	  memory		= 240;
		int	  brainMutation = 3;
		int	  bodyMutation	= 50;
		float exploration = .5;
		float vaporize= .9;
};
