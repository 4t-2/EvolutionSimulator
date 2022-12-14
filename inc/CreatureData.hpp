#pragma once

#include "../lib/AGL/agl.hpp"
#include "NeuralNetwork.hpp"

class CreatureData
{
	private:
		Connection *connection = nullptr;
		int			totalConnections;
		float		sight;
		float		speed;
		float		tough;

	public:
		CreatureData();
		CreatureData(float sight, float speed, float tough, int totalConnections);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		void setConnection(int index, int start, int end, float weight);

		int				   getTotalConnections();
		Connection		  *getConnection();
};
