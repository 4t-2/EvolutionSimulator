#pragma once

#include <AGL/agl.hpp>
#include <IN/intnet.hpp>

class CreatureData
{
	public:
		in::Connection *connection = nullptr; // NOTE maybe store as List<Connection>,
										  // would make mutation easier
		int	  totalConnections;
		float sight; // 0 - 2
		float speed; // 0 - 2
		float size;	 // 0 - 2
		int	  hue;	 // 0 - 359

		float startEnergy;
		float preference; // 1 = plant, -1 = meat
		float metabolism;

		bool useNEAT;
		bool usePG;

		CreatureData();
		CreatureData(float sight, float speed, float size, int hue, int totalConnections);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		void setConnection(int index, int start, int end, float weight);
		void setNetwork(in::NetworkStructure &structure);
};
