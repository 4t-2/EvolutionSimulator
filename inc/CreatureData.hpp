#pragma once

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
		CreatureData(float sight, int hue);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		static int totalSegs(std::vector<SegmentData> &segs);
};
