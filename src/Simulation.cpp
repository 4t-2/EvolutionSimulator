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

	CreatureData creatureData(0, 0, 0, 2);

	creatureData.setConnection(0, CONSTANT_INPUT, RIGHT_OUTPUT, 1);
	creatureData.setConnection(1, CONSTANT_INPUT, FOWARD_OUTPUT, 0.5);

	this->addCreature(&creatureData);

	creatureData.setConnection(0, CONSTANT_INPUT, LEFT_OUTPUT, 1);
	creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 0.5);

	this->addCreature(&creatureData);

	creatureBuffer[0].setPosition({(float)size.x / 2, (float)size.y / 2});
	creatureBuffer[1].setPosition({((float)size.x / 2) + 100, ((float)size.y / 2) + 100});

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

void Simulation::mutateData(unsigned char *buffer, int length, int chance)
{
	for (int i = 0; i < length; i++)
	{
		for (int x = 0; x < 8; x++)
		{
			int mutation = (((float)rand() / (float)RAND_MAX) * chance);
			buffer[i]	 = buffer[i] ^ ((mutation == 0) << x);
		}
	}

	return;
}

void Simulation::addCreature(CreatureData *creatureData)
{
	bool alreadyExists;

	for (int i = 0; i < maxCreatures; i++)
	{
		alreadyExists = false;

		for (int x = 0; x < existingCreatures->getLength(); x++)
		{
			if (&creatureBuffer[i] == existingCreatures->get(x))
			{
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists)
		{
			existingCreatures->add(&creatureBuffer[i]);
			creatureBuffer[i].setup(creatureData);
			break;
		}
	}
}

void Simulation::killCreature(Creature *creature)
{
	creature->clear();

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		if (existingCreatures->get(i) == creature)
		{
			existingCreatures->pop(i);
			break;
		}
	}

	return;
}

void Simulation::updateCreatures()
{
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		existingCreatures->get(i)->updateNetwork(food, totalFood, existingCreatures, size);
		existingCreatures->get(i)->updateActions(food);
	}

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		Creature *eatingCreature = existingCreatures->get(i);

		if (eatingCreature->getEating())
		{
			for (int x = 0; x < existingCreatures->getLength(); x++)
			{
				Creature *eatenCreature = existingCreatures->get(x);

				if (eatingCreature == eatenCreature)
				{
					continue;
				}

				float distance = (eatenCreature->getPosition() - eatingCreature->getPosition()).length();

				if (distance < 25)
				{
					eatenCreature->setHealth(eatenCreature->getHealth() - 1);
					printf("health now %f\n", eatenCreature->getHealth());
				}
			}
		}
	}

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		Creature *creature = existingCreatures->get(i);

		if (creature->getHealth() <= 0)
		{
			this->killCreature(creature);
			i--;
		}
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
