#include "../inc/Creature.hpp"

float hypotenuse(agl::Vec<float, 2>xy)
{
	return sqrt((xy.x * xy.x) + (xy.y * xy.y));
}

Creature::Creature()
{
	Connection connection[TOTAL_CONNECTIONS];

	connection[0].startNode = 0;
	connection[0].endNode	= 11;
	connection[0].weight	= 1;

	connection[1].startNode = 5;
	connection[1].endNode	= 13;
	connection[1].weight	= 1;

	connection[2].startNode = 5;
	connection[2].endNode	= 14;
	connection[2].weight	= -1;

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
	network = new NeuralNetwork(17, 11, connection, TOTAL_CONNECTIONS);

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
	agl::Vec<float, 2> foodOffset;
	foodDistance = 9999999;

	for (int i = 0; i < totalFood; i++)
	{
		if (!food[i].exists)
		{
			continue;
		}

		agl::Vec<float, 2> newOffset   = position - food[i].position;
		float			   newDistance = hypotenuse(newOffset);

		if (newDistance < foodDistance)
		{
			foodDistance = newDistance;
			foodOffset	 = newOffset;
			closestFood	 = &food[i];
		}
	}

	float foodAngle = atan(foodOffset.y / foodOffset.x) - rotation;

	if (foodOffset.y < 0 && foodOffset.x > 0)
	{
		foodAngle += 3.14159;
	}

	if (foodOffset.y > 0 && foodOffset.x > 0)
	{
		foodAngle -= 3.14159;
	}

	network->setInputNode(0, 1);
	network->setInputNode(5, foodAngle / 3.14159);

	network->update();

	return;
}

void Creature::updateActions(Food *food)
{
	velocity = {0, 0};

	float speed = 0;

	if (network->getNode(TOTAL_INPUT + 0).value > 0)
	{
		speed += network->getNode(0).value * 2.5;
	}

	if (network->getNode(TOTAL_INPUT + 1).value > 0)
	{
		speed -= network->getNode(0).value * 2.5;
	}

	if (network->getNode(TOTAL_INPUT + 2).value > 0)
	{
		rotation += 0.05 * network->getNode(13).value;
	}

	if (network->getNode(TOTAL_INPUT + 3).value > 0)
	{
		rotation -= 0.05 * network->getNode(14).value;
	}

	if (network->getNode(TOTAL_INPUT + 4).value > 0)
	{
		eating = true;
	}

	if (network->getNode(TOTAL_INPUT + 5).value > 0)
	{
		layingEgg = true;
	}

	velocity.x = cos(rotation) * speed;
	velocity.y = sin(rotation) * speed;

	position.x += velocity.x;
	position.y += velocity.y;

	if (foodDistance < 25)
	{
		closestFood->exists = false;
		closestFood			= &food[0];
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

Food *Creature::getClosestFood()
{
	return closestFood;
}

bool Creature::getEating()
{
	return eating;
}

bool Creature::getLayingEgg()
{
	return layingEgg;
}
