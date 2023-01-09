#pragma once

#include "../lib/AGL/agl.hpp"
#include "NeuralNetwork.hpp"

#define EXTRA_BYTES 4

class CreatureData
{
	private:
		Connection *connection = nullptr;
		int			totalConnections;
		float		sight; // 0 - 2
		float		speed; // 0 - 2
		float		size;  // 0 - 2
		int			hue;   // 0 - 359

	public:
		CreatureData();
		CreatureData(float sight, float speed, float size, int hue, int totalConnections);
		CreatureData(const CreatureData &creatureData);
		~CreatureData();

		void operator=(CreatureData &creatureData);

		void setConnection(int index, int start, int end, float weight);

		void setSight(float sight);
		void setSpeed(float speed);
		void setSize(float size);
		void setHue(int hue);

		int			getTotalConnections();
		Connection *getConnection();
		float		getSight();
		float		getSpeed();
		float		getSize();
		int			getHue();
};
