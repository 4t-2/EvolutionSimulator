#pragma once

#include "../lib/AGL/agl.hpp"
#include "NeuralNetwork.hpp"

class CreatureData
{
	private:
		Connection *connection = nullptr;
		int			totalConnections;
		float		sight; // 0 - 2
		float		speed; // 0 - 2
		float		size; // 0 - 2

	public:
		CreatureData();
		CreatureData(float sight, float speed, float size, int totalConnections);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		void setConnection(int index, int start, int end, float weight);

		int			getTotalConnections();
		Connection *getConnection();
		float getSight();
		float getSpeed();
		float getSize();
};
