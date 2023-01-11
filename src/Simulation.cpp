#include "../inc/Simulation.hpp"

#include <cstdlib>
#include <thread>

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

Simulation::Simulation(SimulationRules simulationRules)
{
	this->simulationRules = simulationRules;

	srand(time(NULL));

	creatureBuffer	  = new Creature[simulationRules.maxCreatures];
	existingCreatures = new List<Creature *>(simulationRules.maxCreatures);

	eggBuffer	 = new Egg[simulationRules.maxEggs];
	existingEggs = new List<Egg *>(simulationRules.maxEggs);

	foodBuffer	 = new Food[simulationRules.maxFood];
	existingFood = new List<Food *>(simulationRules.maxFood);

	foodGrid	 = new Grid<Food *>(simulationRules.gridResolution, simulationRules.maxFood);
	creatureGrid = new Grid<Creature *>(simulationRules.gridResolution, simulationRules.maxCreatures);

	int connections = 15;

	for (int i = 0; i < 30; i++)
	{
		CreatureData creatureData(1, 1, 1, 0, connections);

		creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
		creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 1);
		creatureData.setConnection(2, CONSTANT_INPUT, LAYEGG_OUTPUT, 1);
		// creatureData.setConnection(3, FOOD_ROTATION, LEFT_OUTPUT, 1);
		// creatureData.setConnection(4, FOOD_ROTATION, RIGHT_OUTPUT, -1);

		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addCreature(creatureData, position);
	}

	for (int i = 0; i < simulationRules.maxFood; i++)
	{
		foodBuffer[i].id = i;

		this->addFood({(float)rand() / (float)RAND_MAX * simulationRules.size.x,
					   (float)rand() / (float)RAND_MAX * simulationRules.size.y});
	}

	return;
}

void Simulation::destroy()
{
	delete foodGrid;
	delete creatureGrid;

	delete existingCreatures;
	delete existingEggs;
	delete existingFood;

	delete[] creatureBuffer;
	delete[] eggBuffer;
	delete[] foodBuffer;
}

Buffer Simulation::creatureDataToBuffer(CreatureData &creatureData)
{
	Buffer buffer((creatureData.getTotalConnections() * 3) + EXTRA_BYTES);

	buffer.data[0] = 255 * (creatureData.getSight() / 2);
	buffer.data[1] = 255 * (creatureData.getSpeed() / 2);
	buffer.data[2] = 255 * (creatureData.getSize() / 2);
	buffer.data[3] = 255 * (creatureData.getHue() / 359.);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		buffer.data[(i * 3) + 0 + EXTRA_BYTES] = creatureData.getConnection()[i].startNode;
		buffer.data[(i * 3) + 1 + EXTRA_BYTES] = creatureData.getConnection()[i].endNode;
		buffer.data[(i * 3) + 2 + EXTRA_BYTES] = 127 * (creatureData.getConnection()[i].weight + 1);
	}

	return buffer;
}

CreatureData Simulation::bufferToCreatureData(Buffer buffer)
{
	CreatureData creatureData((buffer.data[0] * 2) / 255.,	  //
							  (buffer.data[1] * 2) / 255.,	  //
							  (buffer.data[2] * 2) / 255.,	  //
							  (buffer.data[3] * 359.) / 255., //
							  (buffer.size - EXTRA_BYTES) / 3);

	for (int i = 0; i < creatureData.getTotalConnections(); i++)
	{
		creatureData.setConnection(i,												   // id
								   buffer.data[(i * 3) + 0 + EXTRA_BYTES],			   // start
								   buffer.data[(i * 3) + 1 + EXTRA_BYTES],			   // end
								   (buffer.data[(i * 3) + 2 + EXTRA_BYTES] / 127.) - 1 // weight
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

	for (int i = 0; i < simulationRules.maxCreatures; i++)
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
			creatureBuffer[i].setup(creatureData, &simulationRules);
			creatureBuffer[i].setPosition(position);
			creatureBuffer[i].setRotation(((float)rand() / (float)RAND_MAX) * PI * 2);

			break;
		}
	}
}

void Simulation::removeCreature(Creature *creature)
{
	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		if (existingCreatures->get(i) == creature)
		{
			existingCreatures->pop(i);
			break;
		}
	}

	creature->clear();

	return;
}

void Simulation::addEgg(CreatureData &creatureData, agl::Vec<float, 2> position)
{
	bool alreadyExists;

	for (int i = 0; i < simulationRules.maxEggs; i++)
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

	for (int i = 0; i < simulationRules.maxFood; i++)
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
			foodBuffer[i].energy   = simulationRules.foodEnergy;

			agl::Vec<int, 2> gridPosition = foodGrid->toGridPosition(position, simulationRules.size);

			foodGrid->addData(gridPosition, &foodBuffer[i]);

			break;
		}
	}
}

void Simulation::removeFood(Food *food)
{
	for (int i = 0; i < existingFood->getLength(); i++)
	{
		if (existingFood->get(i) == food)
		{
			agl::Vec<int, 2> gridPosition = foodGrid->toGridPosition(food->position, simulationRules.size);

			foodGrid->removeData(gridPosition, food);
			existingFood->pop(i);

			break;
		}
	}

	return;
}

void threadedUpdateNetworks(List<Creature *> *existingCreatures, Grid<Food *> *foodGrid, Grid<Creature *> *creatureGrid,
							int start, int end)
{
	for (int i = start; i < end; i++)
	{
		existingCreatures->get(i)->updateNetwork(foodGrid, creatureGrid);
	}
}

void Simulation::updateNetworks()
{
	std::thread thread1(threadedUpdateNetworks, existingCreatures, foodGrid, creatureGrid, 0,
						existingCreatures->getLength() / 2);
	std::thread thread2(threadedUpdateNetworks, existingCreatures, foodGrid, creatureGrid,
						existingCreatures->getLength() / 2, existingCreatures->getLength());

	thread1.join();
	thread2.join();
}

void mutate(CreatureData *creatureData, int bodyMutation, int networkCycles)
{
	Buffer buf(EXTRA_BYTES);
	buf.data[0] = 255 * (creatureData->getSight() / 2);
	buf.data[1] = 255 * (creatureData->getSpeed() / 2);
	buf.data[2] = 255 * (creatureData->getSize() / 2);
	buf.data[3] = 255 * (creatureData->getHue() / 359.);

	Simulation::mutateBuffer(&buf, bodyMutation);

	creatureData->setSight((buf.data[0] * 2) / 255.);
	creatureData->setSpeed((buf.data[1] * 2) / 255.);
	creatureData->setSize((buf.data[2] * 2) / 255.);
	creatureData->setHue((buf.data[3] * 359.) / 255.);

	for (int i = 0; i < networkCycles; i++)
	{
		int			nonExistIndex = -1;
		Connection *connection	  = creatureData->getConnection();

		for (int i = 0; i < creatureData->getTotalConnections(); i++)
		{
			if (!connection[i].exists)
			{
				nonExistIndex = i;
				break;
			}
		}

		int max = 3;

		if (nonExistIndex == -1)
		{
			max = 1;
		}

		int type = round((rand() / (float)RAND_MAX) * max);

		// 0 - mutate weight
		// 1 - remove connection
		// 2 - Add node
		// 3 - Add connection

		if (type == 0)
		{
			int index = round((rand() / (float)RAND_MAX) * (creatureData->getTotalConnections() - 1));
			int start = connection[index].startNode;
			int end	  = connection[index].endNode;

			connection[index].weight = ((rand() / (float)RAND_MAX) * 2) - 1;
		}
		else if (type == 1)
		{
			int index = round((rand() / (float)RAND_MAX) * (creatureData->getTotalConnections() - 1));

			connection[index].exists = false;
		}
		else if (type == 2)
		{
			int node = -1;

			for (int x = (TOTAL_INPUT + TOTAL_OUTPUT); x < TOTAL_NODES; x++)
			{
				node = x;

				for (int i = 0; i < creatureData->getTotalConnections(); i++)
				{
					if (!connection[i].exists)
					{
						continue;
					}

					if (connection[i].startNode == node || connection[i].endNode == node)
					{
						node = -1;
					}
				}

				if (node != -1)
				{
					break;
				}
			}

			if (node != -1)
			{
				int index = round((rand() / (float)RAND_MAX) * (creatureData->getTotalConnections() - 1));

				connection[nonExistIndex].exists	= true;
				connection[nonExistIndex].startNode = node;
				connection[nonExistIndex].endNode	= connection[index].endNode;
				connection[nonExistIndex].weight	= 1;

				connection[index].endNode = node;
			}
		}
		else if (type == 3)
		{
			List<int> hiddenNodes(creatureData->getTotalConnections());

			for (int i = 0; i < creatureData->getTotalConnections(); i++)
			{
				if (!connection[i].exists)
				{
					continue;
				}

				if (connection[i].startNode < (TOTAL_INPUT + TOTAL_OUTPUT))
				{
					if (hiddenNodes.find(connection[i].startNode) != -1)
					{
						hiddenNodes.add(connection[i].startNode);
					}
				}
			}

			int startNode = round((rand() / (float)RAND_MAX) * (TOTAL_INPUT + hiddenNodes.getLength() - 1));
			int endNode	  = round((rand() / (float)RAND_MAX) * (TOTAL_OUTPUT + hiddenNodes.getLength() - 1));

			if (startNode >= TOTAL_INPUT)
			{
				startNode -= TOTAL_INPUT;
				startNode = hiddenNodes.get(startNode);
			}

			if (endNode >= TOTAL_OUTPUT)
			{
				endNode -= TOTAL_OUTPUT;
				endNode = hiddenNodes.get(startNode);
			}
			else
			{
				endNode += TOTAL_INPUT;
			}

			connection[nonExistIndex].exists	= true;
			connection[nonExistIndex].startNode = startNode;
			connection[nonExistIndex].endNode	= endNode;
			connection[nonExistIndex].weight	= ((rand() / (float)RAND_MAX) * 2) - 1;
		}
	}
}

void Simulation::updateSimulation()
{
	creatureGrid->clear();

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		Creature *creature = existingCreatures->get(i);

		creature->updateActions();

		creature->setGridPosition(foodGrid->toGridPosition(creature->getPosition(), simulationRules.size));

		creatureGrid->addData(creature->getGridPosition(), creature);
	}

	for (int i = 0; i < existingCreatures->getLength(); i++)
	{
		Creature *creature = existingCreatures->get(i);

		// egg laying
		if (creature->getLayingEgg())
		{
			Creature *eggLayer = creature;

			if (eggLayer->getEnergy() > (60 * eggLayer->getSize()))
			{
				eggLayer->setEnergy(eggLayer->getEnergy() - (60 * eggLayer->getSize()));

				CreatureData creatureData = eggLayer->getCreatureData();

				mutate(&creatureData, 100, 1);

				this->addEgg(creatureData, eggLayer->getPosition());
			}
		}

		// creature eating
		if (creature->getEating())
		{
			Creature *eatingCreature = creature;

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
		if (creature->getEnergy() <= 0)
		{
			creature->setHealth(creature->getHealth() - 1);
			creature->setEnergy(0);
		}

		// age damage
		if (creature->getLifeLeft() < 0)
		{
			creature->setHealth(creature->getHealth() - 1);
		}

		// killing creature
		if (creature->getHealth() <= 0)
		{
			this->removeCreature(creature);
			i--;
		}

		List<Food *> *foodList = foodGrid->getList(creature->getGridPosition());

		for (int x = 0; x < foodList->getLength(); x++)
		{
			Food *food = foodList->get(x);

			agl::Vec<float, 2> offset = creature->getPosition() - food->position;

			if (offset.length() < (creature->getRadius() + 10) && creature->getEating())
			{
				creature->setEnergy(creature->getEnergy() + food->energy);

				this->removeFood(food);
				x--;
			}
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

	// adding more food
	int max = std::min(simulationRules.maxFood, int(simulationRules.maxFood * (200. / existingCreatures->getLength())));
	printf("%d\n", max);
	if (existingFood->getLength() < max)
	{
		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addFood(position);
	}
}

void Simulation::update()
{
	this->updateSimulation();
	this->updateNetworks();
}

Creature *Simulation::getCreatureBuffer()
{
	return creatureBuffer;
}

int Simulation::getMaxCreatures()
{
	return simulationRules.maxCreatures;
}

List<Creature *> *Simulation::getExistingCreatures()
{
	return existingCreatures;
}

agl::Vec<float, 2> Simulation::getSize()
{
	return simulationRules.size;
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
	return simulationRules.maxFood;
}
