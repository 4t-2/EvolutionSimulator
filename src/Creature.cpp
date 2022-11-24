#include "../inc/Creature.hpp"

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

float vectorAngle(agl::Vec<float, 2> vec)
{
	float angle = atan(vec.x / vec.y);

	if (vec.y < 0)
	{
		angle *= -1;

		if (vec.x > 0)
		{
			angle = PI - angle;
		}
		else
		{
			angle = -(PI + angle);
		}
	}

	return angle;
}

float loop(float min, float max, float value)
{
	return value - (max + abs(min)) * int(value / max);
}

Creature::Creature()
{
	Connection connection[TOTAL_CONNECTIONS];

	connection[0].startNode = CONSTANT_INPUT;
	connection[0].endNode	= LEFT_OUTPUT;
	connection[0].weight	= 0.5;

	connection[1].startNode = LEFT_OUTPUT;
	connection[1].endNode	= RIGHT_OUTPUT;
	connection[1].weight	= 1;

	connection[2].startNode = RIGHT_OUTPUT;
	connection[2].endNode	= X_INPUT;
	connection[2].weight	= 1;

	// INPUT
	// constant
	// x pos
	// y pos
	// rotation
	// speed
	// Ray[i] distance to object
	// Ray[i] object type (-1 = creature, 0 = nothing, 1 = food)
	//
	// OUTPUT
	// Move foward
	// Move backward
	// Turn right
	// Turn left
	// Eat
	// Lay egg
	network = new NeuralNetwork(TOTAL_NODES, 5 + RAY_TOTAL, connection, TOTAL_CONNECTIONS);

	return;
}

Creature::Creature(char data[TOTAL_CONNECTIONS * 3])
{
	Connection connection[TOTAL_CONNECTIONS];

	for (int i = 0; i < TOTAL_CONNECTIONS; i++)
	{
		connection[i].startNode = data[(i * 3) + 0];
		connection[i].endNode	= data[(i * 3) + 1];
		connection[i].weight	= (float)data[(i * 3) + 2] / 127;
	}

	// INPUT
	// constant
	// x pos
	// y pos
	// rotation
	// speed
	// Ray[i] distance to object
	// Ray[i] object type (-1 = creature, 0 = nothing, 1 = food)
	//
	// OUTPUT
	// Move foward
	// Move backward
	// Turn right
	// Turn left
	// Eat
	// Lay egg
	network = new NeuralNetwork(TOTAL_NODES, 5 + RAY_TOTAL, connection, TOTAL_CONNECTIONS);

	return;
}

void Creature::setPosition(agl::Vec<float, 2> position)
{
	this->position = position;

	return;
}

void Creature::setWorldSize(agl::Vec<float, 2> worldSize)
{
	this->worldSize = worldSize;

	return;
}

void Creature::updateNetwork(Food *food, int totalFood)
{
	for (int x = 0; x < RAY_TOTAL; x++)
	{
		float nearestDistance = RAY_LENGTH;

		for (int i = 0; i < totalFood; i++)
		{
			agl::Vec<float, 2> offset	= position - food[i].position;
			float			   distance = offset.length();

			if (distance > nearestDistance)
			{
				continue;
			}

			float foodRotation	= vectorAngle(offset);
			float creatureAngle = rotation;
			float rayAngle		= (((float)x / (RAY_TOTAL - 1)) * PI) - (PI / 2);

			rayAngle -= creatureAngle;

			float angleDifference	 = loop(-PI, PI, foodRotation - rayAngle);
			float maxAngleDifference = (PI / RAY_TOTAL) / 2;

			if (angleDifference < maxAngleDifference && angleDifference > -maxAngleDifference)
			{
				nearestDistance = distance;
			}
		}

		network->setInputNode((x + 5), (RAY_LENGTH - nearestDistance) / RAY_LENGTH);
	}

	network->setInputNode(0, 1);

	network->update();

	return;
}

void Creature::updateActions(Food *food)
{
	velocity = {0, 0};

	float speed = 0;

	if (network->getNode(FOWARD_OUTPUT).value > 0)
	{
		speed += network->getNode(FOWARD_OUTPUT).value * 2.5;
	}

	if (network->getNode(BACKWARD_OUTPUT).value > 0)
	{
		speed -= network->getNode(BACKWARD_OUTPUT).value * 2.5;
	}

	if (network->getNode(RIGHT_OUTPUT).value > 0)
	{
		rotation += 0.05 * network->getNode(RIGHT_OUTPUT).value;
	}

	if (network->getNode(LEFT_OUTPUT).value > 0)
	{
		rotation -= 0.05 * network->getNode(LEFT_OUTPUT).value;
	}

	if (network->getNode(EAT_OUTPUT).value > 0)
	{
		eating = true;
	}

	if (network->getNode(LAYEGG_OUTPUT).value > 0)
	{
		layingEgg = true;
	}

	rotation = loop(-PI, PI, rotation);

	velocity.x = cos(rotation - (PI / 2)) * speed;
	velocity.y = sin(rotation - (PI / 2)) * speed;

	position.x += velocity.x;
	position.y += velocity.y;

	return;
}

void Creature::saveData(char buffer[TOTAL_CONNECTIONS * 3])
{
	for (int i = 0; i < TOTAL_CONNECTIONS; i++)
	{
		if (network->getConnection(i).valid)
		{
			buffer[(i * 3) + 0] = network->getConnection(i).startNode;
			buffer[(i * 3) + 1] = network->getConnection(i).endNode;
			buffer[(i * 3) + 2] = 127 * network->getConnection(i).weight;
		}
		else
		{

			buffer[(i * 3) + 0] = 0;
			buffer[(i * 3) + 1] = 0;
			buffer[(i * 3) + 2] = 0;
		}
	}
}

void Creature::mutateData(char buffer[TOTAL_CONNECTIONS * 3])
{
	for (int i = 0; i < TOTAL_CONNECTIONS * 3; i++)
	{
		int x	  = (((float)rand() / (float)RAND_MAX) * 8);
		buffer[i] = buffer[i] ^ (1 << x);
	}

	return;
}

NeuralNetwork Creature::getNeuralNetwork()
{
	return *network;
}

agl::Vec<float, 2> Creature::getPosition()
{
	return position;
}

float Creature::getRotation()
{
	return -rotation * 180 / 3.14159;
}

bool Creature::getEating()
{
	return eating;
}

bool Creature::getLayingEgg()
{
	return layingEgg;
}
