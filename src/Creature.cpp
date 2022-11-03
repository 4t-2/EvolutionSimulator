#include "../inc/Creature.hpp"

Creature::Creature()
{
	Connection connection[2];

	connection[0].startNode = 0;
	connection[0].endNode	= 3;
	connection[0].weight	= 1;

	connection[1].startNode = 0;
	connection[1].endNode	= 4;
	connection[1].weight	= -0.1;

	network = new NeuralNetwork(5, 2, connection, 2);

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

void Creature::update(Food *food, int totalFood)
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
		foodAngle -= 3.14159;
	}

	if (foodOffset.y > 0 && foodOffset.x > 0)
	{
		foodAngle += 3.14159;
	}

	network->setInputNode(0, foodAngle / 3.14159);

	network->update();

	velocity = {0, 0};

	float speed = 2.5;

	rotation += 0.05 * network->getNode(3).value;

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
