#include "../inc/Creature.hpp"

#define TOTAL_CONNECTIONS 3

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
	// nearest food rotation offset
	// nearest food distance
	// is there any food
	// nearest creature rotation offset
	// nearest creature distance
	// is there any creature
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

void Creature::setPosition(agl::Vec2f position)
{
	this->position = position;

	return;
}

void Creature::setWorldSize(agl::Vec2f worldSize)
{
	this->worldSize = worldSize;

	return;
}

void Creature::updateActions(Food *food, int totalFood)
{
	agl::Vec2f foodOffset;
	float	   foodDistance = 9999999;

	for (int i = 0; i < totalFood; i++)
	{
		if (!food[i].exists)
		{
			continue;
		}

		agl::Vec2f newOffset   = {position.x - food[i].position.x, position.y - food[i].position.y};
		float	   newDistance = sqrt((newOffset.x * newOffset.x) + (newOffset.y * newOffset.y));

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

	velocity = {0, 0};

	float speed = 0;

	if(network->getNode(11).value > 0)
	{
		speed += network->getNode(0).value * 2.5;
	}

	if(network->getNode(12).value > 0)
	{
		speed -= network->getNode(0).value * 2.5;
	}

	if(network->getNode(13).value > 0)
	{
		rotation += 0.05 * network->getNode(13).value;
	}

	if(network->getNode(14).value > 0)
	{
		rotation -= 0.05 * network->getNode(14).value;
	}

	if(network->getNode(15).value > 0)
	{
		// not done yet
	}

	if(network->getNode(16).value > 0)
	{
		// not done yet
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

agl::Vec2f Creature::getPosition()
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
