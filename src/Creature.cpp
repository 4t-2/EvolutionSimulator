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
	this->life	 = 60 * 60 * 10 * sizeData * sizeData * sizeData;

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

	in::NetworkStructure structure(this->creatureData.totalConnections, 8, 0, 3, connection);

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
			for (int i = 0; i < network->structure.totalOutputNodes; i++)
			{
				shift[i] = (((rand() / (float)RAND_MAX) * 2) - 1) * simulationRules->exploration;
			}
		}

		for (int i = 0; i < network->structure.totalInputNodes; i++)
		{
			memory[memSlot].state[i] = network->inputNode[i]->value;
			if (std::isnan(memory[memSlot].state[i]))
			{
				std::cout << life << " nan " << i << '\n';
			}
		}

		if (std::isnan(reward))
		{
			std::cout << "reward" << '\n';
		}
		memory[memSlot].reward = reward;
		reward				   = 0;

		for (int i = 0; i < network->structure.totalOutputNodes; i++)
		{
			memory[memSlot].action[i] = network->outputNode[i].value;
			if (std::isnan(memory[memSlot].action[i]))
			{
				std::cout << life << " nan  action " << i << '\n';
			}
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
			for (int x = 0; x < network->structure.totalInputNodes; x++)
			{
				network->setInputNode(x, memory[i].state[x]);
			}

			network->update();

			std::vector<float> target(network->structure.totalOutputNodes);

			for (int x = 0; x < network->structure.totalOutputNodes; x++)
			{
				target[x] = memory[i].action[x];
			}

			network->calcGradients(&gradients, target);
		}

		network->learningRate = simulationRules->learningRate;
		network->applyGradients(gradients, loss, simulationRules->memory);
	}

	int node = 2;

	network->setInputNode(0, 1);
	network->setInputNode(1, sin((maxLife - life) / 20.));

	for (int i = 0; i < segments.size(); i++)
	{
		if (segments[i]->rootConnect != nullptr)
		{
			network->setInputNode(node, segments[i]->getJointAngle() / (PI / 2));
			node++;
			network->setInputNode(node, segments[i]->motor / (PI / 2));
			node++;
		}
	}

	network->update();

	for (int i = 0; i < network->structure.totalOutputNodes; i++)
	{
		network->outputNode[i].value += shift[i];
		network->outputNode[i].value = std::clamp<float>(network->outputNode[i].value, -1, 1);
	}
}

void Creature::updateActions()
{
	int node = 0;

	for (auto &seg : segments)
	{
		if (seg->rootConnect != nullptr)
		{
			float ang = seg->getJointAngle();
			float net = network->outputNode[node].value * (PI / 2);

			// float net = sin(frame / 20.);
			// std::cout << ang << '\n';

			float diff = ang - net;
			// std::cout << diff << '\n';

			seg->motor = ((1. / 20) * diff);
			// std::cout << agl::radianToDegree(joint[i].getAngle()) << '\n';

			node++;
		}
	}

	life--;

	return;
}
