#include "../inc/Simulation.hpp"

Simulation::Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood)
{
	this->size		   = size;
	this->maxCreatures = totalCreatures;
	this->totalFood	   = totalFood;

	srand(time(NULL));

	creatureBuffer	  = new Creature[this->maxCreatures];
	existingCreatures = new List<Creature *>(this->maxCreatures);

	food = new Food[totalFood];

	creatureBuffer[0].setPosition({(float)size.x / 2, (float)size.y / 2});
	creatureBuffer[0].setWorldSize({size.x, size.y});

	creatureBuffer[1].setPosition({((float)size.x / 2) + 100, ((float)size.y / 2) + 100});
	creatureBuffer[1].setWorldSize({size.x, size.y});

	for (int i = 0; i < this->maxCreatures; i++)
	{
		existingCreatures->add(&creatureBuffer[i]);
	}

	for (int i = 0; i < totalFood; i++)
	{
		food[i].position.x = (float)rand() / (float)RAND_MAX * size.x;
		food[i].position.y = (float)rand() / (float)RAND_MAX * size.y;
	}

	return;
}

void Simulation::destroy()
{
	delete existingCreatures;
	delete[] creatureBuffer;
	delete[] food;
}

void killCreature(Creature *creature)
{
	return;
}

void Simulation::updateCreatures()
{
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		existingCreatures->get(i)->updateNetwork(food, totalFood, existingCreatures);
		existingCreatures->get(i)->updateActions(food);
	}
}

void Simulation::updateFood()
{
	for (int i = 0; i < totalFood; i++)
	{
		for (int x = 0; x < maxCreatures; x++)
		{
			if (food[i].exists)
			{
				agl::Vec<float, 2> offset = creatureBuffer[x].getPosition() - food[i].position;

				if (offset.length() < 10 && creatureBuffer[x].getEating())
				{
					food[i].exists = false;
					printf("creature %d ate food %d\n", x, i);
				}
			}
		}
	}
}

Creature *Simulation::getCreatureBuffer()
{
	return creatureBuffer;
}

int Simulation::getMaxCreatures()
{
	return maxCreatures;
}

Food *Simulation::getFood()
{
	return food;
}

int Simulation::getTotalFood()
{
	return totalFood;
}

List<Creature *> *Simulation::getExistingCreatures()
{
	return existingCreatures;
}

agl::Vec<float, 2> Simulation::getSize()
{
	return size;
}
