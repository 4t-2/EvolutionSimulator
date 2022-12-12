#pragma once

#include "NeuralNetwork.hpp"
#include "../lib/AGL/agl.hpp"

class CreatureData
{
	private:
		agl::Vec<float, 2> position;
		Connection		  *connection = nullptr;
		int				   totalConnections;
		float			   sight;
		float			   speed;
		float			   tough;

	public:
		CreatureData();
		CreatureData(float sight, float speed, float tough, int totalConnections);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

			void setConnection(int index, int start, int end, float weight);
		void setPosition(agl::Vec<float, 2> position);

		agl::Vec<float, 2> getPosition();
		int				   getTotalConnections();
		Connection		  *getConnection();
};
