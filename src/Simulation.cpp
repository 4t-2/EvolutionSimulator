#include "../inc/Simulation.hpp"

Simulation::Simulation(agl::Vec<float, 2> size, int totalCreatures, int totalFood, int maxEggs)
{
	this->size		   = size;
	this->maxCreatures = totalCreatures;
	this->totalFood	   = totalFood;
	this->maxEggs	   = maxEggs;

	srand(time(NULL));

	creatureBuffer	  = new Creature[this->maxCreatures];
	existingCreatures = new List<Creature *>(this->maxCreatures);

	eggBuffer	 = new Egg[this->maxEggs];
	existingEggs = new List<Egg *>(this->maxEggs);

	food = new Food[totalFood];

	CreatureData creatureData(0, 0, 0, 2);

	creatureData.setConnection(0, CONSTANT_INPUT, RIGHT_OUTPUT, 1);
	creatureData.setConnection(1, CONSTANT_INPUT, FOWARD_OUTPUT, 0.5);

	this->addCreature(creatureData);

	creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
	creatureData.setConnection(1, CONSTANT_INPUT, LAYEGG_OUTPUT, 0.5);

	this->addCreature(creatureData);

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
	delete existingEggs;
	delete[] creatureBuffer;
	delete[] eggBuffer;
	delete[] food;
}

Buffer Simulation::creatureDataToBuffer(CreatureData &creatureData)
{
	Buffer buffer(creatureData.getTotalConnections() * 3);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		buffer.data[i] = creatureData.getConnection()[i].startNode;
		buffer.data[i] = creatureData.getConnection()[i].endNode;
		buffer.data[i] = 127 * creatureData.getConnection()[i].weight;
	}

	return buffer;
}

CreatureData Simulation::bufferToCreatureData(Buffer buffer)
{
	CreatureData creatureData(0, 0, 0, buffer.size / 3);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		creatureData.setConnection(i, buffer.data[(i * 3) + 0], buffer.data[(i * 3) + 1], buffer.data[(i * 3) + 2]);
	}

	return creatureData;
}

void Simulation::mutateBuffer(Buffer *buffer, int chance)
{
	for (int i = 0; i < buffer->size; i++)
	{
		for (int x = 0; x < 8; x++)
		{
			int mutation	= (((float)rand() / (float)RAND_MAX) * chance);
			buffer->data[i] = buffer->data[i] ^ ((mutation == 0) << x);
		}
	}

	return;
}

void Simulation::addCreature(CreatureData &creatureData)
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

void Simulation::addEgg(CreatureData &creatureData)
{
	bool alreadyExists;

	for (int i = 0; i < maxEggs; i++)
	{
		alreadyExists = false;

		for (int x = 0; x < existingEggs->getLength(); x++)
		{
			if (&eggBuffer[i] == existingEggs->get(x))
			{
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists)
		{
			existingEggs->add(&eggBuffer[i]);
			eggBuffer[i].setup(creatureData);
			break;
		}
	}
}

void Simulation::update()
{
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		existingCreatures->get(i)->updateNetwork(food, totalFood, existingCreatures, size);
		existingCreatures->get(i)->updateActions(food);
	}

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		if (existingCreatures->get(i)->getLayingEgg())
		{
			Creature *eggLayer = existingCreatures->get(i);

			if (eggLayer->getEnergy() > 60)
			{
				eggLayer->setEnergy(eggLayer->getEnergy() - 60);
				CreatureData creatureData = eggLayer->saveData();
				this->addEgg(creatureData);
			}
		}

		if (existingCreatures->get(i)->getEating())
		{
			Creature *eatingCreature = existingCreatures->get(i);

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

	for (int i = 0; i < existingEggs->getLength(); i++)
	{
		existingEggs->get(i)->update();
		if(existingEggs->get(i)->getTimeLeft() <= 0)
		{
			CreatureData creatureData = existingEggs->get(i)->getCreatureData();
			this->addCreature(creatureData);
			existingEggs->get(i)->clear();
			existingEggs->pop(i);
		}
	}

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

List<Egg *> *Simulation::getExistingEggs()
{
	return existingEggs;
}

Egg *Simulation::getEggBuffer()
{
	return eggBuffer;
}
