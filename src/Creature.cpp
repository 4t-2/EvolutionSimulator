#include "../inc/Creature.hpp"

Creature::Creature()
{
	Connection connection[2];

	connection[0].startNode = 0;
	connection[0].endNode	= 3;
	connection[0].weight	= 0.1;

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
	network->setInputNode(0, (position.x / worldSize.x * 2)-1);

	network->update();

	velocity = {0, 0};

	float speed = 2.5;

	// if (network->getNode(2).value > 0.5)
	// {
	// 	speed = 2.5;
	// }

	if (network->getNode(3).value > 0.5)
	{
		rotation += 0.05;
	}

	if (network->getNode(4).value > 0.5)
	{
		rotation -= 0.05;
	}

	velocity.x = cos(rotation) * speed;
	velocity.y = sin(rotation) * speed;

	position.x += velocity.x;
	position.y += velocity.y;

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
