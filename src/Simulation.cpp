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

	Connection connection[TOTAL_CONNECTIONS];
	connection[0].startNode = CONSTANT_INPUT;
	connection[0].endNode	= RIGHT_OUTPUT;
	connection[0].weight	= 1;

	connection[1].startNode = CONSTANT_INPUT;
	connection[1].endNode	= FOWARD_OUTPUT;
	connection[1].weight	= 0.5;

	this->addCreature(connection);

	connection[0].endNode = LEFT_OUTPUT;

	this->addCreature(connection);

	this->killCreature(&creatureBuffer[0]);

	this->addCreature(connection);
	
	printf("length %d\n", existingCreatures->getLength());

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

void Simulation::addCreature(Connection connection[TOTAL_CONNECTIONS])
{
	bool alreadyExists;

	printf("start\n");

	for (int i = 0; i < maxCreatures; i++)
	{
		alreadyExists = false;

		printf("i %d\n", i);
		for (int x = 0; x < existingCreatures->getLength(); x++)
		{
			printf("x %d\n", x);
			if (&creatureBuffer[i] == existingCreatures->get(x))
			{
				printf("already exist\n");
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists)
		{
			existingCreatures->add(&creatureBuffer[i]);
			creatureBuffer[i].setup(connection);
			break;
		}
	}

	printf("end\n");
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
