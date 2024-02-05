#include "../inc/Creature.hpp"
#include "../inc/Buffer.hpp"
#include <thread>

Creature::Creature() : Entity<PhysicsObj>(exists, position)
{
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

	this->simulationRules = simulationRules;

	this->creatureData = creatureData;

	sight	 = creatureData.sight;
	speed	 = creatureData.speed;
	sizeData = creatureData.size;

	PhysicsObj::setup({0, 0}, {sizeData * 25, sizeData * 25}, 4);

	hue = creatureData.hue;

	// sight = 1;
	// speed = 1;
	// size = 1;

	this->rayLength = RAY_LENGTH * sight;

	this->maxForce	  = 1.5 * speed;
	this->maxRotation = 0.05 * speed;

	this->health = 100 * sizeData * sizeData * sizeData;
	this->life	 = 60 * 60 * sizeData * sizeData * sizeData;

	this->maxEnergy = 100 * sizeData * sizeData * sizeData;
	this->maxHealth = 100 * sizeData * sizeData * sizeData;
	this->maxLife	= 60 * 60 * sizeData * sizeData * sizeData;

	this->maxBiomass	= 100 * sizeData * sizeData * sizeData;
	this->biomass		= 0;
	this->energyDensity = 0.0;

	// this->radius = 12.5 * size;

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

	creatureRelPos = {0, rayLength};
	foodRelPos	   = {0, rayLength};
	meatRelPos	   = {0, rayLength};

	std::vector<in::Connection> connection(creatureData.connection,
										   creatureData.connection + creatureData.totalConnections);

	in::NetworkStructure structure(this->creatureData.totalConnections, TOTAL_INPUT, TOTAL_HIDDEN, TOTAL_OUTPUT,
								   connection);

	network = new in::NeuralNetwork(structure);

	network->setActivation(in::tanh);
	network->learningRate = .1;
}

void Creature::clear()
{
	network->destroy();
	delete network;

	position = {0, 0};
	velocity = {0, 0};
	force	 = {0, 0};
	rotation = 0;
	// radius	  = 0;
	network	  = nullptr;
	eating	  = false;
	layingEgg = false;
	sight	  = 0;
	speed	  = 0;
	sizeData  = 0;
	energy	  = 0;
	health	  = 0;

	return;
}

Creature::~Creature()
{
	this->clear();
}

float closerObject(agl::Vec<float, 2> offset, float nearestDistance)
{
	return nearestDistance;
}

void Creature::learnBrain(SimulationRules &simRules)
{
}

void Creature::updateNetwork()
{
	if (creatureData.usePG)
	{
		int memSlot = (maxLife - life) % simulationRules->memory;

		if (memSlot == 0)
		{
			for (int i = 0; i < TOTAL_OUTPUT; i++)
			{
				shift[i] = (((rand() / (float)RAND_MAX) * 2) - 1) * simulationRules->exploration;
			}
		}

		for (int i = 0; i < TOTAL_INPUT; i++)
		{
			memory[memSlot].state[i] = network->inputNode[i]->value;
		}

		for (int i = 0; i < TOTAL_OUTPUT; i++)
		{

			network->outputNode[i].value += shift[i];
			network->outputNode[i].value = std::clamp<float>(network->outputNode[i].value, -1, 1);

			memory[memSlot].action[i] = network->outputNode[i].value;
		}
	}

	if ((maxLife - life) % simulationRules->memory == 0 && maxLife != life && creatureData.usePG)
	{
		float loss = 0;

		for (int x = simulationRules->memory - 1; x >= 0; x--)
		{
			for (int y = 1; (x + y) < simulationRules->memory; y++)
			{
				float old = memory[x].reward;
				memory[x].reward += memory[x + y].reward * std::pow(simulationRules->vaporize, y);
			}

			loss += memory[x].reward;
		}

		loss /= simulationRules->memory;

		int oldLoss = loss;
		loss -= baseline;

		baseline = oldLoss;

		std::vector<float> gradients;

		network->setupGradients(&gradients);

		for (int i = 0; i < simulationRules->memory; i++)
		{
			for (int x = 0; x < TOTAL_INPUT; x++)
			{
				network->setInputNode(x, memory[i].state[x]);
			}

			network->update();

			std::vector<float> target(TOTAL_OUTPUT);

			for (int x = 0; x < TOTAL_OUTPUT; x++)
			{
				target[x] = memory[i].action[x];
			}

			network->calcGradients(&gradients, target);
		}

		network->learningRate = simulationRules->learningRate;
		network->applyGradients(gradients, loss, simulationRules->memory);
	}

	network->setInputNode(CONSTANT_INPUT, 1);

	network->setInputNode(X_INPUT, ((position.x / simulationRules->size.x) * 2) - 1);
	network->setInputNode(Y_INPUT, ((position.y / simulationRules->size.y) * 2) - 1);

	network->setInputNode(ROTATION_INPUT, rotation / PI);

	network->setInputNode(SPEED_INPUT, velocity.length());

	network->setInputNode(CREATURE_PREFERENCE, 0);

	network->setInputNode(CREATURE_DISTANCE, 1 - (creatureRelPos.distance / rayLength));
	network->setInputNode(CREATURE_ROTATION, loop(-PI, PI, creatureRelPos.rotation) / PI);

	network->setInputNode(FOOD_DISTANCE, 1 - (foodRelPos.distance / rayLength));
	network->setInputNode(FOOD_ROTATION, loop(-PI, PI, foodRelPos.rotation) / PI);

	network->setInputNode(MEAT_DISTANCE, 1 - (meatRelPos.distance / rayLength));
	network->setInputNode(MEAT_ROTATION, loop(-PI, PI, meatRelPos.rotation) / PI);

	creatureRelPos = {0, rayLength};
	foodRelPos	   = {0, rayLength};
	meatRelPos	   = {0, rayLength};

	network->setInputNode(ENERGY_INPUT, energy / maxEnergy);
	network->setInputNode(HEALTH_INPUT, health / maxHealth);
	network->setInputNode(LIFE_INPUT, (float)life / maxLife);

	network->update();

	return;
}

void Creature::updateActions()
{

	float moveForce = 0;

	// moveForce += 1 * maxForce;

	if (network->getNode(FOWARD_OUTPUT).value > 0)
	{
		moveForce += network->getNode(FOWARD_OUTPUT).value * maxForce;
	}

	if (network->getNode(RIGHT_OUTPUT).value > 0)
	{
		angularAcceleration += maxRotation * network->getNode(RIGHT_OUTPUT).value;
	}

	if (network->getNode(LEFT_OUTPUT).value > 0)
	{
		angularAcceleration -= maxRotation * network->getNode(LEFT_OUTPUT).value;
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

	if (biomass > 0)
	{
		biomass -= metabolism;
		energy += energyDensity * metabolism;

		if (biomass < 0)
		{
			biomass = 0;
		}
	}

	// energy loss
	energy -=
		(sight + (moveForce * moveForce * sizeData * sizeData * sizeData)) * simulationRules->energyCostMultiplier;

	life--;

	acceleration.x += cos(rotation - (PI / 2)) * moveForce * invMass;
	acceleration.y += sin(rotation - (PI / 2)) * moveForce * invMass;

	for (auto seg : segments)
	{
		if(seg->rootConnect == nullptr)
		{
			continue;
		}
		float ang = seg->getJointAngle();
		// float net = network->outputNode[i].value * (PI / 2);

		float net = sin(life / 20.);
		// std::cout << ang << '\n';

		float diff = ang - net;
		// std::cout << diff << '\n';

		seg->motor = ((1. / 20) * diff);
		// std::cout << agl::radianToDegree(joint[i].getAngle()) << '\n';
	}

	return;
}
