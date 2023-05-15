#include "../inc/Creature.hpp"

Creature::Creature() : phy::Circle(*(agl::Circle *)0)
{
	mass = 1;
	return;
}

void Creature::setup(CreatureData &creatureData, SimulationRules *simulationRules)
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

	existing = true;

	this->simulationRules = simulationRules;

	this->gridPosition = {0, 0};

	this->creatureData = creatureData;

	sight = creatureData.sight;
	speed = creatureData.speed;
	size  = creatureData.size;

	hue = creatureData.hue;

	// sight = 1;
	// speed = 1;
	// size = 1;

	this->rayLength = RAY_LENGTH * sight;

	this->maxForce	  = 1.5 * speed;
	this->maxRotation = 0.05 * speed;

	this->health = 100 * size * size * size;
	this->life	 = 60 * 60 * size * size * size;

	this->maxEnergy = 100 * size * size * size;
	this->maxHealth = 100 * size * size * size;
	this->maxLife	= 60 * 60 * size * size * size;

	this->maxBiomass	= 100 * size * size * size;
	this->biomass		= 0;
	this->energyDensity = 0.0;

	this->radius = 12.5 * size;

	this->eggHealthCost = (this->maxHealth / 2);
	this->eggEnergyCost = (this->maxEnergy / 10);
	this->eggTotalCost	= eggHealthCost + eggEnergyCost;
	this->eggDesposit	= 0;

	this->energy = creatureData.startEnergy;

	int xOffset = (roundUp(rayLength / ((float)simulationRules->size.x / simulationRules->gridResolution.x), 2) / 2);
	int yOffset = (roundUp(rayLength / ((float)simulationRules->size.y / simulationRules->gridResolution.y), 2) / 2);

	startGridOffset.x = -xOffset;
	startGridOffset.y = -yOffset;
	endGridOffset.x	  = xOffset;
	endGridOffset.y	  = yOffset;

	std::vector<in::Connection> connection(creatureData.connection,
										   creatureData.connection + creatureData.totalConnections);

	in::NetworkStructure structure(this->creatureData.totalConnections, TOTAL_INPUT, TOTAL_HIDDEN, TOTAL_OUTPUT,
								   connection);

	network = new in::NeuralNetwork(structure);
}

void Creature::clear()
{
	network->destroy();
	delete network;

	existing  = false;
	position  = {0, 0};
	velocity  = {0, 0};
	force	  = {0, 0};
	rotation  = 0;
	radius	  = 0;
	network	  = nullptr;
	eating	  = false;
	layingEgg = false;
	sight	  = 0;
	speed	  = 0;
	size	  = 0;
	energy	  = 0;
	health	  = 0;

	return;
}

Creature::~Creature()
{
	if (existing)
	{
		this->clear();
	}
}

float closerObject(agl::Vec<float, 2> offset, float nearestDistance)
{
	return nearestDistance;
}

void Creature::updateNetwork(Grid<Food *> *foodGrid, Grid<Creature *> *creatureGrid, Grid<Meat *> *meatGrid)
{
	network->setInputNode(CONSTANT_INPUT, 1);

	network->setInputNode(X_INPUT, ((position.x / simulationRules->size.x) * 2) - 1);
	network->setInputNode(Y_INPUT, ((position.y / simulationRules->size.y) * 2) - 1);

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
	// 		float rayAngle		   = (((float)x / (RAY_TOTAL - 1)) * PI)
	// - (PI / 2);
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

	float creatureDistance = rayLength;
	float creatureRotation = 0;
	float foodDistance	   = rayLength;
	float foodRotation	   = 0;

	network->setInputNode(CREATURE_PREFERENCE, 0);

	creatureGrid->updateElements(gridPosition, startGridOffset, endGridOffset, [&](Creature *creature) {
		if (creature == this)
		{
			return;
		}

		agl::Vec<float, 2> offset	= position - creature->position;
		float			   distance = offset.length();

		if (distance > creatureDistance)
		{
			return;
		}

		creatureRotation = vectorAngle(offset) + rotation;
		creatureDistance = distance;

		network->setInputNode(CREATURE_PREFERENCE, creature->preference);
	});

	network->setInputNode(CREATURE_DISTANCE, 1 - (creatureDistance / rayLength));
	network->setInputNode(CREATURE_ROTATION, loop(-PI, PI, creatureRotation) / PI);

	foodGrid->updateElements(gridPosition, startGridOffset, endGridOffset, [&](Food *food) {
		agl::Vec<float, 2> offset	= position - food->position;
		float			   distance = offset.length();

		if (distance > foodDistance)
		{
			return;
		}

		foodRotation = vectorAngle(offset) + rotation;
		foodDistance = distance;
	});

	network->setInputNode(FOOD_DISTANCE, 1 - (foodDistance / rayLength));
	network->setInputNode(FOOD_ROTATION, loop(-PI, PI, foodRotation) / PI);

	float meatDistance = rayLength;
	float meatRotation = 0;

	meatGrid->updateElements(gridPosition, startGridOffset, endGridOffset, [&](Meat *meat) {
		agl::Vec<float, 2> offset	= position - meat->position;
		float			   distance = offset.length();

		if (distance > meatDistance)
		{
			return;
		}

		meatRotation = vectorAngle(offset) + rotation;
		meatDistance = distance;
	});

	network->setInputNode(MEAT_DISTANCE, 1 - (meatDistance / rayLength));
	network->setInputNode(MEAT_ROTATION, loop(-PI, PI, meatRotation) / PI);

	network->setInputNode(ENERGY_INPUT, energy / maxEnergy);
	network->setInputNode(HEALTH_INPUT, health / maxHealth);
	network->setInputNode(LIFE_INPUT, (float)life / maxLife);

	network->update();

	return;
}

void Creature::updateActions(float energyCostMultiplier)
{
	float moveForce = 0;

	if (network->getNode(FOWARD_OUTPUT).value > 0)
	{
		moveForce += network->getNode(FOWARD_OUTPUT).value * maxForce;
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

	if (biomass > 0)
	{
		biomass -= metabolism;
		energy += energyDensity * metabolism;
	}
	else
	{
		biomass = 0;
	}

	// energy loss
	energy -= (sight + (moveForce * moveForce * size * size * size)) * energyCostMultiplier;

	life--;

	force.x = cos(rotation - (PI / 2)) * moveForce;
	force.y = sin(rotation - (PI / 2)) * moveForce;

	// add air resistance

	float dragCoeficient = .1;

	float velMag = velocity.length();

	agl::Vec<float, 2> velNor;

	if (velMag == 0)
	{
		velNor = {1, 0};
	}
	else
	{
		velNor = velocity.normalized();
	}

	agl::Vec<float, 2> drag = (velNor * (velMag * velMag * dragCoeficient)) * (1. / 1);

	force = force - drag;

	this->update();

	return;
}
