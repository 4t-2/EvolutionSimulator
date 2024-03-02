#pragma once

#include "Serializer.hpp"
#include <AGL/agl.hpp>
#include <IN/intnet.hpp>

class SegmentData
{
	public:
		agl::Vec<float, 2>		 size;
		std::vector<SegmentData> branch;
};

class CreatureData
{
	public:
		in::NetworkStructure *netStr = nullptr;

		float sight; // 0 - 2
		int	  hue;	 // 0 - 359

		float startEnergy;
		float preference; // 1 = plant, -1 = meat
		float metabolism;

		bool useNEAT;
		bool usePG;

		std::vector<SegmentData> sd;

		CreatureData();
		CreatureData(float sight, int hue, std::vector<SegmentData> &segs, std::vector<in::Connection> &cons, int maxCon);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		static int totalSegJoints(std::vector<SegmentData> &segs);
};

template <typename T, typename U> void recurse(T processor, agl::Vec<U, 2> &v, std::string name = "null")
{
	processor.process(name, v);

	RECSER(v.x);
	RECSER(v.y);
}

template <typename T> void recurse(T processor, SegmentData &v, std::string name = "null")
{
	processor.process(name, v);

	RECSER(v.size);
	RECSER(v.branch);
}
