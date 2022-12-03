#include "../inc/Simulation.hpp"

Simulation::Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood)
{
	this->size			 = size;
	this->totalCreatures = totalCreatures;
	this->totalFood		 = totalFood;

	srand(time(NULL));

	creature = new Creature[totalCreatures];

	food = new Food[totalFood];

	creature[0].setPosition({(float)size.x / 2, (float)size.y / 2});
	creature[0].setWorldSize({size.x, size.y});

	creature[1].setPosition({((float)size.x / 2) + 100, ((float)size.y / 2) + 100});
	creature[1].setWorldSize({size.x, size.y});

	for (int i = 0; i < totalFood; i++)
	{
		food[i].position.x = (float)rand() / (float)RAND_MAX * size.x;
		food[i].position.y = (float)rand() / (float)RAND_MAX * size.y;
	}

	return;
}

void Simulation::destroy()
{
	delete[] creature;
	delete[] food;
}

void Simulation::updateCreatures()
{
	for (int i = 0; i < totalCreatures; i++)
	{
		creature[i].updateNetwork(food, totalFood, creature, totalCreatures);
		creature[i].updateActions(food);
	}
}

void Simulation::updateFood()
{
	for (int i = 0; i < totalFood; i++)
	{
		for (int x = 0; x < totalCreatures; x++)
		{
			if (food[i].exists)
			{
				agl::Vec<float, 2> offset = creature[x].getPosition() - food[i].position;

				if (offset.length() < 10)
				{
					food[i].exists = false;
					printf("creature %d ate food %d\n", x, i);
				}
			}
		}
	}
}

Creature *Simulation::getCreature()
{
	return creature;
}

Food *Simulation::getFood()
{
	return food;
}

int Simulation::getTotalFood()
{
	return totalFood;
}

int Simulation::getTotalCreatures()
{
	return totalCreatures;
}

agl::Vec<float, 2> Simulation::getSize()
{
	return size;
}
