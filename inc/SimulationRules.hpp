#pragma once

#include "macro.hpp"
#include <AGL/agl.hpp>
#include "Serializer.hpp"

class SimulationRules
{
	public:
		// starting
		agl::Vec<int, 2> size = {10000, 10000};
		agl::Vec<int, 2> gridResolution = {10, 10};
		int				 startingCreatures = 10;

		// variable
		// int preferedCreatures;
		// int penaltyBuffer;
		// int penaltyPeriod;

		int	  foodCap = 1000;
		float energyCostMultiplier = ENERGYCOSTMULTIPLIER;

		int threads = THREADS;

		float learningRate	= .1;
		int	  memory		= 240;
		int	  brainMutation = 3;
		int	  bodyMutation	= 50;
		float exploration = .5;
		float vaporize= .9;
};

template<typename T>
void recurse(T processor, agl::Vec<int, 2> &v, std::string name="null")
{
	processor.process(name, v);

	RECSER(v.x);
	RECSER(v.y);
}

template<typename T>
void recurse(T processor, SimulationRules &s, std::string name="null")
{
	processor.process(name, s);

	RECSER(s.size);
	RECSER(s.gridResolution);
	RECSER(s.startingCreatures);
	RECSER(s.foodCap);
	RECSER(s.energyCostMultiplier);
	RECSER(s.threads);
	RECSER(s.learningRate);
	RECSER(s.memory);
	RECSER(s.brainMutation);
	RECSER(s.bodyMutation);
	RECSER(s.exploration);
	RECSER(s.vaporize);
}
