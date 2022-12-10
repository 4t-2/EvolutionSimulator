#pragma once

#include "../lib/AGL/agl.hpp"
#include "Food.hpp"
#include "List.hpp"
#include "NeuralNetwork.hpp"

#define RAY_TOTAL  10
#define RAY_LENGTH 500

#define TOTAL_INPUT	 (5 + (RAY_TOTAL * 2))
#define TOTAL_HIDDEN 6
#define TOTAL_NODES	 (TOTAL_INPUT + TOTAL_HIDDEN)

#define TOTAL_CONNECTIONS 2

#define CONSTANT_INPUT		   0
#define X_INPUT				   1
#define Y_INPUT				   2
#define ROTATION_INPUT		   3
#define SPEED_INPUT			   4
#define RAYDISTANCESTART_INPUT 5

#define FOWARD_OUTPUT	(TOTAL_INPUT + 0)
#define BACKWARD_OUTPUT (TOTAL_INPUT + 1)
#define RIGHT_OUTPUT	(TOTAL_INPUT + 2)
#define LEFT_OUTPUT		(TOTAL_INPUT + 3)
#define EAT_OUTPUT		(TOTAL_INPUT + 4)
#define LAYEGG_OUTPUT	(TOTAL_INPUT + 5)

class Buffer
{
	public:
		Buffer(int size);
		~Buffer();
		unsigned char *data;
		int			   size;
};

class CreatureData
{
	private:
		Connection *connection;
		int			totalConnections;
		float		sight;
		float		speed;
		float		tough;

	public:
		CreatureData(float sight, float speed, float tough, int totalConnections);
		~CreatureData();

		void setConnection(int index, int start, int end, float weight);

		int			getTotalConnections();
		Connection *getConnection();
};

class Creature
{
	private:
		agl::Vec<float, 2> position = {0, 0};
		agl::Vec<float, 2> velocity = {0, 0};
		float			   rotation = 0;

		float radius;

		NeuralNetwork *network;

		bool eating	   = false;
		bool layingEgg = false;

		float sight;
		float speed;
		float tough;

		// sight + (speed^2)(tough^2)
		float energy = 100;
		float health = 100;

	public:
		float closest;
		float closestAngle;

		Creature();
		~Creature();

		void setup(CreatureData *creatureData);
		void clear();

		void setPosition(agl::Vec<float, 2> position);
		void setVelocity(agl::Vec<float, 2> velocity);
		void setRotation(float rotation);
		void setHealth(float health);

		void updateNetwork(Food *food, int totalFood, List<Creature *> *existingCreatures,
						   agl::Vec<float, 2> worldSize);
		void updateActions(Food *food);

		CreatureData *saveData();

		NeuralNetwork	   getNeuralNetwork();
		agl::Vec<float, 2> getPosition();
		agl::Vec<float, 2> getVelocity();
		float			   getRotation();
		bool			   getEating();
		bool			   getLayingEgg();
		float			   getHealth();
};
