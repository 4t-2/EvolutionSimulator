#include "../inc/Creature.hpp"
#include "../inc/Buffer.hpp"
#include <thread>


Creature::Creature()
{
	return;
}

void Creature::clear()
{
	position = {0, 0};
	velocity = {0, 0};
	force	 = {0, 0};
	rotation = 0;
	// radius	  = 0;
	eating	  = false;
	layingEgg = false;
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
	// if (creatureData.usePG)
	// {
	// 	int memSlot = (maxLife - life) % simulationRules->memory;
	//
	// 	if (memSlot == 0)
	// 	{
	// 		for (int i = 0; i < network->structure.totalOutputNodes; i++)
	// 		{
	// 			shift[i] = (((rand() / (float)RAND_MAX) * 2) - 1) * simulationRules->exploration;
	// 		}
	// 	}
	//
	// 	for (int i = 0; i < network->structure.totalInputNodes; i++)
	// 	{
	// 		memory[memSlot].state[i] = network->inputNode[i]->value;
	// 		if (std::isnan(memory[memSlot].state[i]))
	// 		{
	// 			std::cout << life << " nan " << i << '\n';
	// 		}
	// 	}
	//
	// 	if (std::isnan(reward))
	// 	{
	// 		std::cout << "reward" << '\n';
	// 	}
	// 	memory[memSlot].reward = reward;
	// 	reward				   = 0;
	//
	// 	for (int i = 0; i < network->structure.totalOutputNodes; i++)
	// 	{
	// 		memory[memSlot].action[i] = network->outputNode[i].value;
	// 		if (std::isnan(memory[memSlot].action[i]))
	// 		{
	// 			std::cout << life << " nan  action " << i << '\n';
	// 		}
	// 	}
	// }
	//
	// if ((maxLife - life) % simulationRules->memory == 0 && maxLife != life && creatureData.usePG)
	// {
	// 	float loss = 0;
	//
	// 	for (int x = simulationRules->memory - 1; x >= 0; x--)
	// 	{
	// 		for (int y = 1; (x + y) < simulationRules->memory; y++)
	// 		{
	// 			float old = memory[x].reward;
	// 			memory[x].reward += memory[x + y].reward * std::pow(simulationRules->vaporize, y);
	// 		}
	//
	// 		loss += memory[x].reward;
	// 	}
	//
	// 	loss /= simulationRules->memory;
	//
	// 	int oldLoss = loss;
	// 	loss -= baseline;
	//
	// 	baseline = oldLoss;
	//
	// 	std::vector<float> gradients;
	//
	// 	network->setupGradients(&gradients);
	//
	// 	for (int i = 0; i < simulationRules->memory; i++)
	// 	{
	// 		for (int x = 0; x < network->structure.totalInputNodes; x++)
	// 		{
	// 			network->setInputNode(x, memory[i].state[x]);
	// 		}
	//
	// 		network->update();
	//
	// 		std::vector<float> target(network->structure.totalOutputNodes);
	//
	// 		for (int x = 0; x < network->structure.totalOutputNodes; x++)
	// 		{
	// 			target[x] = memory[i].action[x];
	// 		}
	//
	// 		network->calcGradients(&gradients, target);
	// 	}
	//
	// 	network->learningRate = simulationRules->learningRate;
	// 	network->applyGradients(gradients, loss, simulationRules->memory);
	// }

	/*int node = 2;*/
	/**/
	/*network->setInputNode(0, 1);*/
	/*network->setInputNode(1, sinf((maxLife - life) / 20.f));*/
	/**/
	/*for (int i = 0; i < segments.size(); i++)*/
	/*{*/
	/*	if (segments[i]->rootConnect != nullptr)*/
	/*	{*/
	/*		network->setInputNode(node, segments[i]->getJointAngle() / (float)(PI / 2));*/
	/*		node++;*/
	/*		network->setInputNode(node, segments[i]->motor / (float)(PI / 2));*/
	/*		node++;*/
	/*	}*/
	/*}*/
	/**/
	/*network->update();*/

	// for (int i = 0; i < network->structure.totalOutputNodes; i++)
	// {
	// 	network->outputNode[i].value += shift[i];
	// 	network->outputNode[i].value =
	// std::clamp<float>(network->outputNode[i].value, -1, 1);
	// }
}

void Creature::updateActions()
{
	int node = 0;

	for (auto &seg : segments)
	{
		if (seg->rootConnect != nullptr)
		{
			/*float ang = seg->getJointAngle();*/
			/*float net = network->outputNode[node].value * (float)(PI / 2);*/
			/**/
			/*// float net = sin(frame / 20.);*/
			/*// std::cout << ang << '\n';*/
			/**/
			/*float diff = ang - net;*/
			/*// std::cout << diff << '\n';*/
			/**/
			/*seg->motor = (1 / 6.f * diff) * seg->maxMotor;*/
			/*// std::cout << agl::radianToDegree(joint[i].getAngle()) << '\n';*/
			/**/
			/*node++;*/
		}
	}

	life--;

	return;
}
