#include "../inc/Simulation.hpp"

#include <cstdlib>
#include <thread>

#include <fstream>

#define LOGCREATUREDATA

#define THREADS	  2
#define EATRADIUS 5
#define DAMAGE	  25

#define FOODVOL	 25
#define MEATVOL	 25
#define LEACHVOL .25

#define BITEDELAY 20

// 1 health = 2 energy

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

	srand(0);

	creatureBuffer	  = new Creature[simulationRules.maxCreatures];
	existingCreatures = new List<Creature *>(simulationRules.maxCreatures);

	eggBuffer	 = new Egg[simulationRules.maxEggs];
	existingEggs = new List<Egg *>(simulationRules.maxEggs);

	foodBuffer	 = new Food[simulationRules.maxFood];
	existingFood = new List<Food *>(simulationRules.maxFood);

	foodGrid	 = new Grid<Food *>(simulationRules.gridResolution, simulationRules.maxFood);
	creatureGrid = new Grid<Creature *>(simulationRules.gridResolution, simulationRules.maxCreatures);

	meatBuffer	 = new Meat[MAXMEAT];
	existingMeat = new List<Meat *>(MAXMEAT);
	meatGrid	 = new Grid<Meat *>(simulationRules.gridResolution, MAXMEAT);

	int connections = 15;

	for (int i = 0; i < simulationRules.startingCreatures; i++)
	{
		CreatureData creatureData(.5, .5, 1, 0, connections);

		creatureData.setConnection(0, CONSTANT_INPUT, FOWARD_OUTPUT, 1);
		creatureData.setConnection(1, CONSTANT_INPUT, EAT_OUTPUT, 1);
		creatureData.setConnection(2, CONSTANT_INPUT, LAYEGG_OUTPUT, 1);
		creatureData.setConnection(3, FOOD_ROTATION, LEFT_OUTPUT, 1);
		creatureData.setConnection(4, FOOD_ROTATION, RIGHT_OUTPUT, -1);

		creatureData.preference = 1;

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
	delete meatGrid;

	delete existingCreatures;
	delete existingEggs;
	delete existingFood;
	delete existingMeat;

	delete[] creatureBuffer;
	delete[] eggBuffer;
	delete[] foodBuffer;
	delete[] meatBuffer;

#ifdef LOGCREATUREDATA

	std::fstream cpd("./plot/cpd.txt", std::ios::out);
	std::fstream csigd("./plot/csigd.txt", std::ios::out);
	std::fstream csped("./plot/csped.txt", std::ios::out);
	std::fstream csizd("./plot/csizd.txt", std::ios::out);

	for (int x = 0; x < creaturePopData.size(); x++)
	{
		cpd << x << " " << creaturePopData[x] << "\n";
		csigd << x << " " << creatureSightData[x] << "\n";
		csped << x << " " << creatureSpeedData[x] << "\n";
		csizd << x << " " << creatureSizeData[x] << "\n";
	}

	cpd.close();
	csigd.close();
	csped.close();
	csizd.close();

#endif
}

Buffer Simulation::creatureDataToBuffer(CreatureData &creatureData)
{
	Buffer buffer((creatureData.totalConnections * 3) + EXTRA_BYTES);

	buffer.data[0] = 255 * (creatureData.sight / 2);
	buffer.data[1] = 255 * (creatureData.speed / 2);
	buffer.data[2] = 255 * (creatureData.size / 2);
	buffer.data[3] = 255 * (creatureData.hue / 359.);

	for (int i = 0; i < creatureData.totalConnections; i++)
	{
		buffer.data[(i * 3) + 0 + EXTRA_BYTES] = creatureData.connection[i].startNode;
		buffer.data[(i * 3) + 1 + EXTRA_BYTES] = creatureData.connection[i].endNode;
		buffer.data[(i * 3) + 2 + EXTRA_BYTES] = 127 * (creatureData.connection[i].weight + 1);
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

	for (int i = 0; i < creatureData.totalConnections; i++)
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

		for (int x = 0; x < existingCreatures->length; x++)
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
			creatureBuffer[i].position = position;
			creatureBuffer[i].rotation = ((float)rand() / (float)RAND_MAX) * PI * 2;

			break;
		}
	}
}

void Simulation::removeCreature(Creature *creature)
{
	for (int i = 0; i < existingCreatures->length; i++)
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

		for (int x = 0; x < existingEggs->length; x++)
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
			eggBuffer[i].position = position;
			break;
		}
	}
}

void Simulation::removeEgg(Egg *egg)
{
	egg->clear();

	for (int i = 0; i < existingEggs->length; i++)
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

		for (int x = 0; x < existingFood->length; x++)
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

			foodBuffer[i].nextRandPos(simulationRules.size);

			break;
		}
	}
}

void Simulation::removeFood(Food *food)
{
	for (int i = 0; i < existingFood->length; i++)
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

void Simulation::addMeat(agl::Vec<float, 2> position, float energy)
{
	bool alreadyExists;

	for (int i = 0; i < MAXMEAT; i++)
	{
		alreadyExists = false;

		for (int x = 0; x < existingMeat->length; x++)
		{
			if (&meatBuffer[i] == existingMeat->get(x))
			{
				alreadyExists = true;
				break;
			}
		}

		if (!alreadyExists)
		{
			existingMeat->add(&meatBuffer[i]);
			meatBuffer[i].position = position;
			meatBuffer[i].energy   = energy;

			agl::Vec<int, 2> gridPosition = meatGrid->toGridPosition(position, simulationRules.size);

			meatGrid->addData(gridPosition, &meatBuffer[i]);

			break;
		}
	}
}

void Simulation::addMeat(agl::Vec<float, 2> position)
{
	this->addMeat(position, 60);
}

void Simulation::removeMeat(Meat *meat)
{
	for (int i = 0; i < existingMeat->length; i++)
	{
		if (existingMeat->get(i) == meat)
		{
			agl::Vec<int, 2> gridPosition = meatGrid->toGridPosition(meat->position, simulationRules.size);

			meatGrid->removeData(gridPosition, meat);
			existingMeat->pop(i);

			break;
		}
	}

	return;
}

float mutShift(float f, float max)
{
	float push = ((float)rand() / (float)RAND_MAX);
	push -= .5;
	push /= 2;

	return std::max((float)0, std::min(max, f + push));
}

void mutate(CreatureData *creatureData, int bodyMutation, int networkCycles)
{
	// creatureData->sight = mutShift(creatureData->sight, 4);
	// creatureData->speed = mutShift(creatureData->speed, 4);
	// creatureData->size	= mutShift(creatureData->size, 4);
	// creatureData->hue	= mutShift(creatureData->hue / 60., 359. / 60) * 60;

	Buffer buf(EXTRA_BYTES);
	buf.data[0] = 255 * (creatureData->sight / 2);
	buf.data[1] = 255 * (creatureData->speed / 2);
	buf.data[2] = 255 * (creatureData->size / 2);
	buf.data[3] = 255 * (creatureData->hue / 359.);

	Simulation::mutateBuffer(&buf, bodyMutation);

	creatureData->sight = (buf.data[0] * 2) / 255.;
	creatureData->speed = (buf.data[1] * 2) / 255.;
	creatureData->size	= (buf.data[2] * 2) / 255.;
	creatureData->hue	= (buf.data[3] * 359.) / 255.;

	for (int i = 0; i < networkCycles; i++)
	{
		int nonExistIndex = -1;

		Connection *connection = creatureData->connection;

		for (int i = 0; i < creatureData->totalConnections; i++)
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
			int index = round((rand() / (float)RAND_MAX) * (creatureData->totalConnections - 1));
			int start = connection[index].startNode;
			int end	  = connection[index].endNode;

			connection[index].weight = ((rand() / (float)RAND_MAX) * 4) - 2;
		}
		else if (type == 1)
		{
			int index = round((rand() / (float)RAND_MAX) * (creatureData->totalConnections - 1));

			connection[index].exists = false;
		}
		else if (type == 2)
		{
			int node = -1;

			for (int x = (TOTAL_INPUT + TOTAL_OUTPUT); x < TOTAL_NODES; x++)
			{
				node = x;

				for (int i = 0; i < creatureData->totalConnections; i++)
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
				int index = round((rand() / (float)RAND_MAX) * (creatureData->totalConnections - 1));

				connection[nonExistIndex].exists	= true;
				connection[nonExistIndex].startNode = node;
				connection[nonExistIndex].endNode	= connection[index].endNode;
				connection[nonExistIndex].weight	= 1;

				connection[index].endNode = node;
			}
		}
		else if (type == 3)
		{
			List<int> hiddenNodes(creatureData->totalConnections);

			for (int i = 0; i < creatureData->totalConnections; i++)
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

			int startNode = round((rand() / (float)RAND_MAX) * (TOTAL_INPUT + hiddenNodes.length - 1));
			int endNode	  = round((rand() / (float)RAND_MAX) * (TOTAL_OUTPUT + hiddenNodes.length - 1));

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
			connection[nonExistIndex].weight	= ((rand() / (float)RAND_MAX) * 4) - 2;
		}
	}
}

void correctPosition(phy::Circle &circle, phy::Circle &otherCircle)
{
	agl::Vec<float, 2> circleOffset = otherCircle.position - circle.position;

	float circleDistance = circleOffset.length();

	float circleOverlap = (otherCircle.radius + circle.radius) - circleDistance;

	if (circleOverlap > 0)
	{
		if (circleDistance == 0)
		{
			circleOffset   = {rand() / (float)RAND_MAX - (float).5, rand() / (float)RAND_MAX - (float).5};
			circleDistance = circleOffset.length();
		}

		agl::Vec<float, 2> offsetNormal = circleOffset.normalized();
		agl::Vec<float, 2> pushback		= offsetNormal * circleOverlap;

		float actingMass = circle.mass > otherCircle.mass ? otherCircle.mass : circle.mass;

		circle.posOffset -= pushback * (otherCircle.mass / (circle.mass + otherCircle.mass));
		otherCircle.posOffset += pushback * (circle.mass / (circle.mass + otherCircle.mass));

		circle.force -= pushback * actingMass;
		otherCircle.force += pushback * actingMass;
	}
}

void multithreadedRecurse(int size, std::function<void(int i)> lambda)
{
	auto recurse = [&lambda](int start, int end) {
		for (int i = start; i <= end; i++)
		{
			lambda(i);
		}
	};

	std::thread **thread = new std::thread *[THREADS];

	int i = 0;

	for (i = 0; i < THREADS - 1; i++)
	{
		int start = (size / THREADS) * i;
		int end	  = (size / THREADS) * (i + 1) - 1;

		thread[i] = new std::thread(recurse, start, end);
	}

	int start = (size / THREADS) * i;
	int end	  = size - 1;

	thread[i] = new std::thread(recurse, start, end);

	for (int i = 0; i < THREADS; i++)
	{
		thread[i]->join();
		delete thread[i];
	}

	delete[] thread;
}

agl::Vec<int, 2> indexToPosition(int i, agl::Vec<int, 2> size)
{
	return {i % size.x, i / size.x};
}

template <typename T, typename U> void updatePhysics(agl::Vec<int, 2> gridPosition, Grid<T> *Tgrid, Grid<U> *Ugrid)
{
	List<T> *Tlist = Tgrid->getList(gridPosition);
	List<U> *Ulist = Ugrid->getList(gridPosition);

	for (int x = 0; x < Tlist->length; x++)
	{
		T t = Tlist->get(x);

		for (int y = 0; y < Ulist->length; y++)
		{
			U u = Ulist->get(y);

			correctPosition(*t, *u);
		}

		Ugrid->updateElements(gridPosition, {1, 1}, {1, 1}, [&](U u) { correctPosition(*t, *u); });
		Ugrid->updateElements(gridPosition, {1, 0}, {1, 0}, [&](U u) { correctPosition(*t, *u); });
		Ugrid->updateElements(gridPosition, {0, 1}, {0, 1}, [&](U u) { correctPosition(*t, *u); });
	}
}

template <typename T> void updatePhysics(agl::Vec<int, 2> gridPosition, Grid<T> *Tgrid)
{
	List<T> *list = Tgrid->getList(gridPosition);

	for (int x = 0; x < list->length; x++)
	{
		T t1 = list->get(x);

		for (int y = x + 1; y < list->length; y++)
		{
			correctPosition(*list->get(x), *list->get(y));
		}

		Tgrid->updateElements(gridPosition, {1, 1}, {1, 1}, [&](T t2) { correctPosition(*t1, *t2); });
		Tgrid->updateElements(gridPosition, {1, 0}, {1, 0}, [&](T t2) { correctPosition(*t1, *t2); });
		Tgrid->updateElements(gridPosition, {0, 1}, {0, 1}, [&](T t2) { correctPosition(*t1, *t2); });
	}
}

void Simulation::updateSimulation()
{
	multithreadedRecurse(creatureGrid->size.x, [&](int x) {
		if (x % 2 == 0)
		{
			return;
		}

		for (int y = 0; y < creatureGrid->size.y; y++)
		{
			updatePhysics({x, y}, creatureGrid);
			updatePhysics({x, y}, creatureGrid, foodGrid);
			updatePhysics({x, y}, foodGrid);
			updatePhysics({x, y}, meatGrid);
			updatePhysics({x, y}, meatGrid, foodGrid);
			updatePhysics({x, y}, meatGrid, creatureGrid);
		}
	});

	multithreadedRecurse(creatureGrid->size.x, [&](int x) {
		if (x % 2 != 0)
		{
			return;
		}

		for (int y = 0; y < creatureGrid->size.y; y++)
		{
			updatePhysics(agl::Vec<int, 2>{x, y}, creatureGrid);
			updatePhysics(agl::Vec<int, 2>{x, y}, creatureGrid, foodGrid);
			updatePhysics(agl::Vec<int, 2>{x, y}, foodGrid);
			updatePhysics({x, y}, meatGrid);
			updatePhysics({x, y}, meatGrid, foodGrid);
			updatePhysics({x, y}, meatGrid, creatureGrid);
		}
	});

	// everything else

	creatureGrid->clear();

	for (int i = 0; i < existingCreatures->length; i++)
	{
		Creature *creature = existingCreatures->get(i);

		creature->updateActions();

		creature->gridPosition = foodGrid->toGridPosition(creature->position, simulationRules.size);

		creatureGrid->addData(creature->gridPosition, creature);
	}

	foodGrid->clear();

	for (int i = 0; i < existingFood->length; i++)
	{
		Food *food = existingFood->get(i);

		phy::Circle &circle = *existingFood->get(i);

		float dragCoeficient = .1;

		float velMag = circle.velocity.length();

		agl::Vec<float, 2> velNor;

		if (velMag == 0)
		{
			velNor = {1, 0};
		}
		else
		{
			velNor = circle.velocity.normalized();
		}

		agl::Vec<float, 2> drag = (velNor * (velMag * velMag * dragCoeficient)) * (1. / 1);

		circle.force -= drag;

		if ((food->nextPos - food->position).length() < 50)
		{
			food->nextRandPos(simulationRules.size);
		}

		food->force += (food->nextPos - food->position).normalized() / 100;

		food->update();

		if (food->position.x < 0)
		{
			food->force.x += 1;
		}
		if (food->position.x > simulationRules.size.x)
		{
			food->force.x -= 1;
		}

		if (food->position.y < 0)
		{
			food->force.y += 1;
		}
		if (food->position.y > simulationRules.size.y)
		{
			food->force.y -= 1;
		}

		food->exists = true;

		foodGrid->addData(foodGrid->toGridPosition(food->position, simulationRules.size), food);
	}

	meatGrid->clear();

	for (int i = 0; i < existingMeat->length; i++)
	{
		Meat *meat = existingMeat->get(i);

		phy::Circle &circle = *existingMeat->get(i);

		float dragCoeficient = .1;

		float velMag = circle.velocity.length();

		agl::Vec<float, 2> velNor;

		if (velMag == 0)
		{
			velNor = {1, 0};
		}
		else
		{
			velNor = circle.velocity.normalized();
		}

		agl::Vec<float, 2> drag = (velNor * (velMag * velMag * dragCoeficient)) * (1. / 1);

		circle.force -= drag;

		meat->update();

		meat->exists = true;

		meatGrid->addData(meatGrid->toGridPosition(meat->position, simulationRules.size), meat);
	}

	for (int i = 0; i < existingCreatures->length; i++)
	{
		Creature *creature = existingCreatures->get(i);

		// egg laying
		if (creature->layingEgg)
		{
			if (creature->energy > creature->eggCost)
			{
				creature->incubating = true;
			}
		}

		if (creature->incubating)
		{
			creature->energy -= PREGNANCY_COST;
			creature->eggDesposit += PREGNANCY_COST;

			if (creature->eggDesposit >= creature->eggCost)
			{
				CreatureData creatureData = creature->creatureData;

				mutate(&creatureData, 50, 1);

				creatureData.startEnergy = creature->eggCost - (sizeToHealth(creatureData.size) / 2);

				this->addEgg(creatureData, creature->position);

				creature->incubating  = false;
				creature->eggDesposit = 0;
			}
		}

		// creature eating
		if (creature->eating)
		{
			foodGrid->updateElements(
				foodGrid->toGridPosition(creature->position, simulationRules.size), {-1, -1}, {1, 1}, [&](Food *food) {
					if (!food->exists)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature->position - food->position;

					float angleDiff = offset.angle() - creature->rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature->radius + food->radius + EATRADIUS))
					{
						float energy = foodEnergyDensity * creature->preference;

						creature->energyDensity = ((creature->biomass * creature->energyDensity) + (FOODVOL * energy)) /
												  (creature->biomass + FOODVOL);
						creature->biomass += FOODVOL;

						food->exists = false;
						this->removeFood(food);
					}
				});

			meatGrid->updateElements(
				meatGrid->toGridPosition(creature->position, simulationRules.size), {-1, -1}, {1, 1}, [&](Meat *meat) {
					if (!meat->exists)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature->position - meat->position;

					float angleDiff = offset.angle() - creature->rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature->radius + meat->radius + EATRADIUS))
					{
						float energy = MEATENERGY * (1 - creature->preference);

						creature->energyDensity = ((creature->biomass * creature->energyDensity) + (MEATVOL * energy)) /
												  (creature->biomass + MEATVOL);
						creature->biomass += MEATVOL;

						meat->exists = false;
						this->removeMeat(meat);
					}
				});

			creatureGrid->updateElements(
				creatureGrid->toGridPosition(creature->position, simulationRules.size), {-1, -1}, {1, 1},
				[&](Creature *eatenCreature) {
					if (eatenCreature == creature || eatenCreature->health < 0)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature->position - eatenCreature->position;

					float angleDiff = offset.angle() - creature->rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature->radius + eatenCreature->radius + EATRADIUS))
					{
						if (frame % BITEDELAY == 0)
						{
							float energy = MEATENERGY * (1 - creature->preference);

							creature->energyDensity =
								((creature->biomass * creature->energyDensity) + (LEACHVOL * energy)) /
								(creature->biomass + LEACHVOL);
							creature->biomass += LEACHVOL;

							eatenCreature->health -= DAMAGE;
						}
					}
				});
		}

		// tired creature damage
		if (creature->energy <= 0)
		{
			creature->health--;
			creature->energy = 0;
		}

		// age damage
		if (creature->life < 0)
		{
			creature->health--;
		}

		// killing creature
		if (creature->health <= 0)
		{
			this->addMeat(creature->position, creature->maxHealth / 2);
			this->removeCreature(creature);
			i--;
		}

		List<Food *> *foodList = foodGrid->getList(creature->gridPosition);

		if (creature->energy > creature->maxEnergy)
		{
			creature->energy = creature->maxEnergy;
		}
		if (creature->biomass > creature->maxBiomass)
		{
			creature->biomass = creature->maxBiomass;
		}
	}

	// egg hatching
	for (int i = 0; i < existingEggs->length; i++)
	{
		existingEggs->get(i)->update();
		if (existingEggs->get(i)->timeleft <= 0)
		{
			Egg *hatchedEgg = existingEggs->get(i);

			CreatureData creatureData = hatchedEgg->creatureData;
			this->addCreature(creatureData, hatchedEgg->position);

			removeEgg(hatchedEgg);
		}
	}

	static int penalty = 0;

	int adjustedMaxFood =
		int(simulationRules.maxFood * ((float)simulationRules.preferedCreatures / existingCreatures->length));
	adjustedMaxFood = std::min(adjustedMaxFood, simulationRules.maxFood);

	if (existingCreatures->length > simulationRules.preferedCreatures)
	{
		penalty++;

		if (penalty > ((adjustedMaxFood + simulationRules.penaltyBuffer) * simulationRules.penaltyPeriod))
		{
			penalty = ((adjustedMaxFood + simulationRules.penaltyBuffer) * simulationRules.penaltyPeriod);
		}
	}
	else
	{
		penalty--;

		if (penalty < 0)
		{
			penalty = 0;
		}
	}

	adjustedMaxFood -= (penalty / simulationRules.penaltyPeriod);

	// adding more food
	// int max = std::min(simulationRules.maxFood, adjustedMaxFood);
	int max = simulationRules.maxFood;
	// int max = 0;

	for (; existingFood->length < max;)
	{
		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addFood(position);
	}

	// threaded network update

	multithreadedRecurse(existingCreatures->length,
						 [&](int i) { existingCreatures->get(i)->updateNetwork(foodGrid, creatureGrid, meatGrid); });
}

void Simulation::update()
{
	// this->threadableUpdate();
	this->updateSimulation();

	frame++;

#ifdef LOGCREATUREDATA
	creaturePopData.emplace_back(existingCreatures->length);

	float totSight = 0;
	float totSpeed = 0;
	float totSize  = 0;

	for (int i = 0; i < existingCreatures->length; i++)
	{
		totSight += existingCreatures->get(i)->sight;
		totSpeed += existingCreatures->get(i)->speed;
		totSize += existingCreatures->get(i)->size;
	}

	creatureSightData.emplace_back(totSight / existingCreatures->length);
	creatureSpeedData.emplace_back(totSpeed / existingCreatures->length);
	creatureSizeData.emplace_back(totSize / existingCreatures->length);
#endif
}
