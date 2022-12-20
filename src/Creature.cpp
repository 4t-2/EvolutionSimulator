#include "../inc/Creature.hpp"

Creature::Creature()
{
	return;
}

void Creature::setup(CreatureData &creatureData)
{
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

	this->creatureData = creatureData;
	this->energy	   = 50;

	network =
		new NeuralNetwork(TOTAL_NODES, TOTAL_INPUT, creatureData.getConnection(), creatureData.getTotalConnections());
}

void Creature::clear()
{
	delete network;

	position	 = {0, 0};
	velocity	 = {0, 0};
	acceleration = {0, 0};
	rotation	 = 0;
	radius		 = 0;
	network		 = nullptr;
	eating		 = false;
	layingEgg	 = false;
	sight		 = 0;
	speed		 = 0;
	tough		 = 0;
	energy		 = 100;
	health		 = 100;

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

void Creature::setHealth(float health)
{
	this->health = health;

	return;
}

void Creature::setEnergy(float energy)
{
	this->energy = energy;

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

void Creature::updateNetwork(List<Food *> *existingFood, List<Creature *> *existingCreature,
							 agl::Vec<float, 2> worldSize)
{
	network->setInputNode(CONSTANT_INPUT, 1);

	network->setInputNode(X_INPUT, ((position.x / worldSize.x) * 2) - 1);
	network->setInputNode(Y_INPUT, ((position.y / worldSize.y) * 2) - 1);

	network->setInputNode(ROTATION_INPUT, rotation / PI);

	network->setInputNode(SPEED_INPUT, velocity.length());

	// for (int x = 0; x < RAY_TOTAL; x++)
	// {
	// 	float nearestDistance = RAY_LENGTH;
	// 	int	  type			  = 0;
	//
	// 	for (int i = 0; i < existingFood->getLength(); i++)
	// 	{
	// 		agl::Vec<float, 2> offset	= position -
	// existingFood->get(i)->position; 		float distance =
	// offset.length();
	//
	// 		if (distance > nearestDistance)
	// 		{
	// 			continue;
	// 		}
	//
	// 		float foodRotation	= vectorAngle(offset);
	// 		float creatureAngle = rotation;
	// 		float rayAngle		= (((float)x / (RAY_TOTAL - 1)) * PI) -
	// (PI / 2);
	//
	// 		rayAngle -= creatureAngle;
	//
	// 		float angleDifference	 = loop(-PI, PI, foodRotation -
	// rayAngle); 		float maxAngleDifference = (PI / RAY_TOTAL) / 2;
	//
	// 		if (angleDifference < maxAngleDifference && angleDifference >
	// -maxAngleDifference)
	// 		{
	// 			nearestDistance = distance;
	// 			type			= 1;
	// 		}
	// 	}
	//
	// 	for (int i = 0; i < existingCreature->getLength(); i++)
	// 	{
	// 		if (existingCreature->get(i) == this)
	// 		{
	// 			continue;
	// 		}
	//
	// 		agl::Vec<float, 2> offset	= position -
	// existingCreature->get(i)->getPosition();
	// 		float			   distance = offset.length();
	//
	// 		if (distance > nearestDistance)
	// 		{
	// 			continue;
	// 		}
	//
	// 		float creatureRotation = vectorAngle(offset);
	// 		float creatureAngle	   = rotation;
	// 		float rayAngle		   = (((float)x / (RAY_TOTAL - 1)) * PI) -
	// (PI / 2);
	//
	// 		rayAngle -= creatureAngle;
	//
	// 		float angleDifference	 = loop(-PI, PI, creatureRotation -
	// rayAngle); 		float maxAngleDifference = (PI / RAY_TOTAL) / 2;
	//
	// 		if (angleDifference < maxAngleDifference && angleDifference >
	// -maxAngleDifference)
	// 		{
	// 			nearestDistance = distance;
	// 			type			= -1;
	// 		}
	// 	}
	//
	// 	network->setInputNode((x + 5), (RAY_LENGTH - nearestDistance) /
	// RAY_LENGTH); 	network->setInputNode((x + 5) + RAY_TOTAL, type);
	// }

	float creatureDistance = RAY_LENGTH;
	float creatureRotation = 0;
	float foodDistance	   = RAY_LENGTH;
	float foodRotation	   = 0;

	for (int i = 0; i < existingCreature->getLength(); i++)
	{
		if (existingCreature->get(i) == this)
		{
			continue;
		}

		agl::Vec<float, 2> offset	= position - existingCreature->get(i)->position;
		float			   distance = offset.length();

		if (distance > creatureDistance)
		{
			continue;
		}

		creatureRotation = vectorAngle(offset) + rotation;
		creatureDistance = distance;
	}

	network->setInputNode(CREATURE_DISTANCE, 1 - (creatureDistance / RAY_LENGTH));
	network->setInputNode(CREATURE_ROTATION, creatureRotation / PI);

	for (int i = 0; i < existingFood->getLength(); i++)
	{
		agl::Vec<float, 2> offset	= position - existingFood->get(i)->position;
		float			   distance = offset.length();

		if (distance > foodDistance)
		{
			continue;
		}

		foodRotation = vectorAngle(offset) + rotation;
		foodDistance = distance;
	}

	network->setInputNode(FOOD_DISTANCE, 1 - (foodDistance / RAY_LENGTH));
	network->setInputNode(FOOD_ROTATION, foodRotation / PI);

	network->update();

	return;
}

void Creature::updateActions(Food *food)
{
	acceleration = {0, 0};

	float speed = 0;

	float maxSpeed	  = 1.5;
	float maxRotation = 0.05;

	if (network->getNode(FOWARD_OUTPUT).value > 0)
	{
		speed += network->getNode(FOWARD_OUTPUT).value * maxSpeed;
	}

	if (network->getNode(BACKWARD_OUTPUT).value > 0)
	{
		speed -= network->getNode(BACKWARD_OUTPUT).value * maxSpeed;
	}

	if (network->getNode(RIGHT_OUTPUT).value > 0)
	{
		rotation += maxRotation * network->getNode(RIGHT_OUTPUT).value;
	}

	if (network->getNode(LEFT_OUTPUT).value > 0)
	{
		rotation -= maxRotation * network->getNode(LEFT_OUTPUT).value;
	}

	if (network->getNode(EAT_OUTPUT).value > 0)
	{
		eating = true;
	}
	else
	{
		eating = false;
	}

	if (network->getNode(LAYEGG_OUTPUT).value > 0)
	{
		layingEgg = true;
	}
	else
	{
		layingEgg = false;
	}

	rotation = loop(-PI, PI, rotation);

	energy -= abs(speed) / 50; // needs to be absolute as going backwards would create energy
	energy -= 0.03;

	float			   density		 = 2;
	agl::Vec<float, 2> airResistance = {velocity.x / density, //
										velocity.y / density};

	acceleration.x = cos(rotation - (PI / 2)) * speed;
	acceleration.y = sin(rotation - (PI / 2)) * speed;

	acceleration = acceleration - airResistance;

	velocity.x += acceleration.x;
	velocity.y += acceleration.y;

	position.x += velocity.x;
	position.y += velocity.y;

	return;
}

CreatureData Creature::getCreatureData()
{
	return creatureData;
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

float Creature::getHealth()
{
	return health;
}

float Creature::getEnergy()
{
	return energy;
}
