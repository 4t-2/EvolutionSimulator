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
	connection[0].endNode	= FOWARD_OUTPUT;
	connection[0].weight	= 1;

	connection[1].startNode = CONSTANT_INPUT;
	connection[1].endNode	= LEFT_OUTPUT;
	connection[1].weight	= 1;

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
	network = new NeuralNetwork(TOTAL_NODES, 5 + (RAY_TOTAL*2), connection, TOTAL_CONNECTIONS);

	return;
}

Creature::Creature(unsigned char data[TOTAL_CONNECTIONS * 3])
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
	network = new NeuralNetwork(TOTAL_NODES, 5 + (RAY_TOTAL*2), connection, TOTAL_CONNECTIONS);

	return;
}

Creature::~Creature()
{
	this->clear();
}

void Creature::setPosition(agl::Vec<float, 2> position)
{
	this->position = position;

	return;
}

void Creature::setVelocity(agl::Vec<float, 2> velocity)
{
	this->velocity = velocity;

	return;
}

void Creature::setRotation(float rotation)
{
	this->rotation = rotation;

	return;
}

void Creature::setWorldSize(agl::Vec<float, 2> worldSize)
{
	this->worldSize = worldSize;

	return;
}

bool isVisible()
{
	return false;
}

float closerObject(agl::Vec<float, 2> offset, float nearestDistance)
{
	return nearestDistance;
}

void Creature::updateNetwork(Food *food, int totalFood, List<Creature*> *existingCreature)
{
	for (int x = 0; x < RAY_TOTAL; x++)
	{
		float nearestDistance = RAY_LENGTH;
		int	  type			  = 0;

		for (int i = 0; i < totalFood; i++)
		{
			if(!food[i].exists)
			{
				continue;
			}

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
				type			= 1;
			}
		}

		for (int i = 0; i < existingCreature->getLength(); i++)
		{
			if (existingCreature->get(i) == this)
			{
				continue;
			}

			agl::Vec<float, 2> offset	= position - existingCreature->get(i)->getPosition();
			float			   distance = offset.length();

			if (distance > nearestDistance)
			{
				continue;
			}

			float creatureRotation = vectorAngle(offset);
			float creatureAngle	   = rotation;
			float rayAngle		   = (((float)x / (RAY_TOTAL - 1)) * PI) - (PI / 2);

			rayAngle -= creatureAngle;

			float angleDifference	 = loop(-PI, PI, creatureRotation - rayAngle);
			float maxAngleDifference = (PI / RAY_TOTAL) / 2;

			if (angleDifference < maxAngleDifference && angleDifference > -maxAngleDifference)
			{
				nearestDistance = distance;
				type			= -1;
			}
		}

		network->setInputNode((x + 5), (RAY_LENGTH - nearestDistance) / RAY_LENGTH);
		network->setInputNode((x + 5) + RAY_TOTAL, type);
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

	energy -= speed;

	velocity.x = cos(rotation - (PI / 2)) * speed;
	velocity.y = sin(rotation - (PI / 2)) * speed;

	position.x += velocity.x;
	position.y += velocity.y;

	return;
}

void Creature::saveData(unsigned char buffer[TOTAL_CONNECTIONS * 3])
{
	for (int i = 0; i < TOTAL_CONNECTIONS; i++)
	{
		buffer[(i * 3) + 0] = network->getConnection(i).startNode;
		buffer[(i * 3) + 1] = network->getConnection(i).endNode;
		buffer[(i * 3) + 2] = 127 * network->getConnection(i).weight;
	}
}

void Creature::mutateData(unsigned char buffer[TOTAL_CONNECTIONS * 3], int chance)
{
	for (int i = 0; i < TOTAL_CONNECTIONS * 3; i++)
	{
		for (int x = 0; x < 8; x++)
		{
			int mutation = (((float)rand() / (float)RAND_MAX) * chance);
			buffer[i]	 = buffer[i] ^ ((mutation == 0) << x);
		}
	}

	return;
}

void Creature::clear()
{
	delete network;
	network = nullptr;

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

agl::Vec<float, 2> Creature::getVelocity()
{
	return velocity;
}

float Creature::getRotation()
{
	return rotation;
}

bool Creature::getEating()
{
	return eating;
}

bool Creature::getLayingEgg()
{
	return layingEgg;
}
