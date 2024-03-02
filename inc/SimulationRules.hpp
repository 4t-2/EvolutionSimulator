#pragma once

#include "CreatureData.hpp"
#include "Serializer.hpp"
#include "macro.hpp"
#include <AGL/agl.hpp>

class SimulationRules
{
	public:
		// starting
		agl::Vec<int, 2> size			   = {10000, 10000};
		agl::Vec<int, 2> gridResolution	   = {10, 10};
		int				 startingCreatures = 10;

		// variable
		// int preferedCreatures;
		// int penaltyBuffer;
		// int penaltyPeriod;

		int	  foodCap			   = 1000;
		float energyCostMultiplier = ENERGYCOSTMULTIPLIER;

		int threads = THREADS;

		float					 learningRate  = .1f;
		int						 memory		   = 240;
		int						 brainMutation = 3;
		int						 bodyMutation  = 50;
		float					 exploration   = .5f;
		float					 vaporize	   = .9f;
		std::vector<SegmentData> startBody = {
			 {{24, 24}, {}},
			 {{4, 24}, {}},
		 };
		std::vector<in::Connection> startBrain = {in::Connection(1, 4, 1)};
		int maxConnections;
};

template <typename T> void recurse(T processor, in::Connection &c, std::string name = "null")
{
	processor.process(name, c);

	RECSER(c.startNode);
	RECSER(c.endNode);
	RECSER(c.weight);
	RECSER(c.id);
	RECSER(c.valid);
	RECSER(c.exists);
}

template <typename T> void recurse(T processor, SimulationRules &s, std::string name = "null")
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
	RECSER(s.startBody);
	RECSER(s.startBrain);
	RECSER(s.maxConnections);
}
