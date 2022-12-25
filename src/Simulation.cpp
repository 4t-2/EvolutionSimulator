#include "../inc/Simulation.hpp"

void randomData(Buffer *buffer)
{
	for (int i = 0; i < buffer->size; i++)
	{
		for (int x = 0; x < 8; x++)
		{
			int bit			= (((float)rand() / (float)RAND_MAX) * 2);
			buffer->data[i] = buffer->data[i] | (bit << x);
		}
	}

	return;
}

Simulation::Simulation(agl::Vec<float, 2> size, int maxCreatures, int maxFood, int maxEggs)
{
	this->size		   = size;
	this->maxCreatures = maxCreatures;
	this->maxFood	   = maxFood;
	this->maxEggs	   = maxEggs;

	srand(time(NULL));

	creatureBuffer	  = new Creature[this->maxCreatures];
	existingCreatures = new List<Creature *>(this->maxCreatures);

	eggBuffer	 = new Egg[this->maxEggs];
	existingEggs = new List<Egg *>(this->maxEggs);

	foodBuffer	 = new Food[this->maxFood];
	existingFood = new List<Food *>(this->maxFood);

	int connections = 10;

	for (int i = 0; i < 10; i++)
	{
		Buffer buffer(3 + (connections * 3));
		randomData(&buffer);

		CreatureData creatureData = this->bufferToCreatureData(buffer);

		// CreatureData creatureData(1, 1, 1, 5);

		creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
		creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 1);
		creatureData.setConnection(2, FOOD_ROTATION, LEFT_OUTPUT, 1);
		creatureData.setConnection(3, FOOD_ROTATION, RIGHT_OUTPUT, -1);
		creatureData.setConnection(4, CONSTANT_INPUT, LAYEGG_OUTPUT, 1);

		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * size.x;
		position.y = (rand() / (float)RAND_MAX) * size.y;

		this->addCreature(creatureData, position);
	}

	for (int i = 0; i < this->maxFood; i++)
	{
		this->addFood({(float)rand() / (float)RAND_MAX * size.x, //
					   (float)rand() / (float)RAND_MAX * size.y});
	}

	foodBuffer->position = {((float)size.x / 2) + 100, 100};

	return;
}

void Simulation::destroy()
{
	delete existingCreatures;
	delete existingEggs;
	delete[] creatureBuffer;
	delete[] eggBuffer;
	delete[] foodBuffer;
}

Buffer Simulation::creatureDataToBuffer(CreatureData &creatureData)
{
	Buffer buffer((creatureData.getTotalConnections() * 3) + 3);

	buffer.data[0] = 255 * ((creatureData.getSight()) / 2);
	buffer.data[1] = 255 * ((creatureData.getSpeed()) / 2);
	buffer.data[2] = 255 * ((creatureData.getSize()) / 2);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		buffer.data[(i * 3) + 0 + 3] = creatureData.getConnection()[i].startNode;
		buffer.data[(i * 3) + 1 + 3] = creatureData.getConnection()[i].endNode;
		buffer.data[(i * 3) + 2 + 3] = 127 * (creatureData.getConnection()[i].weight + 1);
	}

	return buffer;
}

CreatureData Simulation::bufferToCreatureData(Buffer buffer)
{
	CreatureData creatureData((buffer.data[0] * 2) / 255., //
							  (buffer.data[1] * 2) / 255., //
							  (buffer.data[2] * 2) / 255., //
							  (buffer.size - 3) / 3);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		creatureData.setConnection(i,										 // id
								   buffer.data[(i * 3) + 0 + 3],			 // start
								   buffer.data[(i * 3) + 1 + 3],			 // end
								   (buffer.data[(i * 3) + 2 + 3] / 127.) - 1 // weight
		);
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

void Simulation::addCreature(CreatureData &creatureData, agl::Vec<float, 2> position)
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
			creatureBuffer[i].setPosition(position);
			creatureBuffer[i].setRotation(((float)rand() / (float)RAND_MAX) * PI * 2);

			break;
		}
	}
}

void Simulation::removeCreature(Creature *creature)
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

void Simulation::addEgg(CreatureData &creatureData, agl::Vec<float, 2> position)
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
			eggBuffer[i].setPosition(position);
			break;
		}
	}
}

void Simulation::removeEgg(Egg *egg)
{
	egg->clear();

	for (int i = 0; i < existingEggs->getLength(); i++)
	{
		if (existingEggs->get(i) == egg)
		{
			existingEggs->pop(i);
			break;
		}
	}

	return;
}

void Simulation::addFood(agl::Vec<float, 2> position)
{
	bool alreadyExists;

	for (int i = 0; i < maxFood; i++)
	{
		alreadyExists = false;

		for (int x = 0; x < existingFood->getLength(); x++)
		{
			if (&foodBuffer[i] == existingFood->get(x))
			{
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists)
		{
			existingFood->add(&foodBuffer[i]);
			foodBuffer[i].position = position;
			foodBuffer[i].energy   = 20;
			break;
		}
	}
}

void Simulation::removeFood(Food *food)
{
	*food = Food{};

	for (int i = 0; i < existingFood->getLength(); i++)
	{
		if (existingFood->get(i) == food)
		{
			existingFood->pop(i);
			break;
		}
	}

	return;
}
void Simulation::update()
{
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		existingCreatures->get(i)->updateNetwork(existingFood, existingCreatures, size);
		existingCreatures->get(i)->updateActions(foodBuffer);
	}

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		// egg laying
		if (existingCreatures->get(i)->getLayingEgg())
		{
			Creature *eggLayer = existingCreatures->get(i);

			if (eggLayer->getEnergy() > (60 * eggLayer->getSize()))
			{
				eggLayer->setEnergy(eggLayer->getEnergy() - (60 * eggLayer->getSize()));

				CreatureData creatureData = eggLayer->getCreatureData();
				Buffer		 buffer		  = creatureDataToBuffer(creatureData);

				mutateBuffer(&buffer, 100);

				CreatureData mutatedData = bufferToCreatureData(buffer);

				this->addEgg(mutatedData, eggLayer->getPosition());
			}
		}

		// creature eating
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

				if (distance < (eatingCreature->getRadius() + eatenCreature->getRadius()))
				{
					eatenCreature->setHealth(eatenCreature->getHealth() - 1);
				}
			}
		}

		// tired creature damage
		if (existingCreatures->get(i)->getEnergy() <= 0)
		{
			existingCreatures->get(i)->setHealth(existingCreatures->get(i)->getHealth() - 1);
			existingCreatures->get(i)->setEnergy(0);
		}

		// age damage
		if (existingCreatures->get(i)->getLifeLeft() < 0)
		{
			existingCreatures->get(i)->setHealth(existingCreatures->get(i)->getHealth() - 1);
		}
	}

	// killing creature
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		Creature *creature = existingCreatures->get(i);

		if (creature->getHealth() <= 0)
		{
			this->removeCreature(creature);
			i--;
		}
	}

	// egg hatching
	for (int i = 0; i < existingEggs->getLength(); i++)
	{
		existingEggs->get(i)->update();
		if (existingEggs->get(i)->getTimeLeft() <= 0)
		{
			Egg *hatchedEgg = existingEggs->get(i);

			CreatureData creatureData = hatchedEgg->getCreatureData();
			this->addCreature(creatureData, hatchedEgg->getPosition());

			removeEgg(hatchedEgg);
		}
	}

	// creature eating food
	for (int i = 0; i < existingFood->getLength(); i++)
	{
		Food *food = existingFood->get(i);

		for (int x = 0; x < existingCreatures->getLength(); x++)
		{
			Creature *creature = existingCreatures->get(x);

			agl::Vec<float, 2> offset = creature->getPosition() - food->position;

			if (offset.length() < (creature->getRadius() + 10) && creature->getEating())
			{
				creature->setEnergy(creature->getEnergy() + food->energy);

				existingFood->pop(i);
				i--;
			}
		}
	}

	// adding more food
	if (existingFood->getLength() < maxFood)
	{

		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * size.x;
		position.y = (rand() / (float)RAND_MAX) * size.y;

		this->addFood(position);
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

Food *Simulation::getFoodBuffer()
{
	return foodBuffer;
}

List<Food *> *Simulation::getExistingFood()
{
	return existingFood;
}

int Simulation::getMaxFood()
{
	return maxFood;
}
