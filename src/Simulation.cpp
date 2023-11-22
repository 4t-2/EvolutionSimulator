#include "../inc/Simulation.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <random>
#include <thread>
#include <type_traits>

#define newEnergyDensity(biomass, bioEnergy, additional, addEnergy) \
	((biomass * bioEnergy) + (additional * addEnergy)) / (biomass + additional)

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

void Simulation::create(SimulationRules simulationRules, int seed)
{
	active = true;
	frame  = 0;

	this->simulationRules = simulationRules;

	srand(seed);

	foodGrid	 = new Grid<Food *>(simulationRules.gridResolution);
	creatureGrid = new Grid<Creature *>(simulationRules.gridResolution);
	meatGrid	 = new Grid<Meat *>(simulationRules.gridResolution);

	in::NetworkStructure basicStructure(TOTAL_INPUT, {}, TOTAL_OUTPUT, false);

	for (int i = 0; i < simulationRules.startingCreatures; i++)
	{
		CreatureData creatureData(1, .5, 1, 0, basicStructure.totalConnections);

		switch (i % 2)
		{
			case 0:
				creatureData.useNEAT = true;
				creatureData.usePG	 = false;
				break;
			case 1:
				creatureData.useNEAT = false;
				creatureData.usePG	 = true;
				break;
			case 2:
				creatureData.useNEAT = true;
				creatureData.usePG	 = true;
				break;
		}

		in::NetworkStructure::randomWeights(basicStructure);
		creatureData.setNetwork(basicStructure);

		creatureData.preference = 1;
		creatureData.metabolism = METABOLISM;

		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addCreature(creatureData, position);
	}

	for (int i = 0; i < foodCap; i++)
	{
		this->addFood({(float)rand() / (float)RAND_MAX * simulationRules.size.x,
					   (float)rand() / (float)RAND_MAX * simulationRules.size.y});
	}

	foodCap = simulationRules.foodCap;

	return;
}

void Simulation::destroy()
{
	active = false;

	env.destroy();

	delete foodGrid;
	delete creatureGrid;
	delete meatGrid;

#ifdef LOGCREATUREDATA

	std::fstream neat("./plot/neat.txt", std::ios::out);
	std::fstream rl("./plot/rl.txt", std::ios::out);
	std::fstream both("./plot/both.txt", std::ios::out);

	for (int x = 0; x < totalNEAT.size(); x++)
	{
		neat << totalNEAT[x] << "\n";
		rl << totalRL[x] << "\n";
		both << totalBOTH[x] << "\n";
	}

	neat.close();
	rl.close();
	both.close();

	totalNEAT.clear();
	totalRL.clear();
	totalBOTH.clear();

#endif
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
	Creature &newCreature = env.addEntity<Creature>();
	newCreature.setup(creatureData, &simulationRules);
	newCreature.position = position;
	newCreature.rotation = ((float)rand() / (float)RAND_MAX) * PI * 2;
}

void Simulation::removeCreature(std::list<EntityData>::iterator creature)
{
	env.removeEntity<Creature>(creature, [&](Creature &creature) {
		agl::Vec<int, 2> gridPosition = creatureGrid->toGridPosition(creature.position, simulationRules.size);
		creatureGrid->removeData(gridPosition, &creature);

		creature.clear();
	});

	return;
}

void Simulation::addEgg(CreatureData &creatureData, agl::Vec<float, 2> position)
{
	Egg &newEgg = env.addEntity<Egg>();
	newEgg.setup(creatureData);
	newEgg.position = position;
}

void Simulation::removeEgg(std::list<EntityData>::iterator egg)
{
	env.removeEntity<Egg>(egg, [](Egg &egg) { egg.clear(); });

	return;
}

void Simulation::addFood(agl::Vec<float, 2> position)
{
	Food &newFood	 = env.addEntity<Food>();
	newFood.position = position;
	newFood.energy	 = simulationRules.foodEnergy;

#ifdef ACTIVEFOOD
	newFood.nextRandPos(simulationRules.size);
#endif
}

void Simulation::removeFood(Food *food)
{
	std::list<EntityData>::iterator iterator;

	env.view<Food>([&](auto, auto it) {
		if (it->data == food)
		{
			iterator = it;
		}
	});

	removeFood(iterator);
}

void Simulation::removeFood(std::list<EntityData>::iterator food)
{
	env.removeEntity<Food>(food, [&](Food &food) {
		agl::Vec<int, 2> gridPosition = foodGrid->toGridPosition(food.position, simulationRules.size);
		foodGrid->removeData(gridPosition, &food);
	});

	return;
}

void Simulation::addMeat(agl::Vec<float, 2> position, float energy)
{
	Meat &newMeat	  = env.addEntity<Meat>();
	newMeat.position  = position;
	newMeat.energyVol = (float)energy / meatEnergyDensity;
	newMeat.radius	  = (energy / 50.) * 5;
	newMeat.rotation  = ((float)rand() / (float)RAND_MAX) * 360;
	newMeat.lifetime  = newMeat.energyVol * 288;
}

void Simulation::addMeat(agl::Vec<float, 2> position)
{
	this->addMeat(position, 50);
}

void Simulation::removeMeat(std::list<EntityData>::iterator meat)
{
	env.removeEntity<Meat>(meat, [&](Meat &meat) {
		agl::Vec<int, 2> gridPosition = meatGrid->toGridPosition(meat.position, simulationRules.size);
		meatGrid->removeData(gridPosition, &meat);
	});

	return;
}

void Simulation::removeMeat(Meat *meat)
{
	std::list<EntityData>::iterator iterator;

	env.view<Meat>([&](auto, auto it) {
		if (it->data == meat)
		{
			iterator = it;
		}
	});

	removeMeat(iterator);
}

float mutShift(float f, float min, float max)
{
	float push = ((float)rand() / (float)RAND_MAX);
	push -= .5;
	push /= 2;

	return std::max((float)min, std::min(max, f + push));
}

void mutate(CreatureData *creatureData, int bodyMutation, int networkCycles)
{
	// creatureData->sight		 = mutShift(creatureData->sight, .5, 4);
	// creatureData->speed		 = mutShift(creatureData->speed, .5, 4);
	// creatureData->size		 = mutShift(creatureData->size, .5, 4);
	// creatureData->hue		 = mutShift(creatureData->hue / 60., 0, 359. /
	// 60)
	// * 60; creatureData->preference = mutShift(creatureData->preference, 0, 1);
	// creatureData->metabolism = mutShift(creatureData->metabolism, 0, 2);

	Buffer buf(EXTRA_BYTES);
	buf.data[0] = 255 * (creatureData->sight / 2);
	buf.data[1] = 255 * (creatureData->speed / 2);
	buf.data[2] = 255 * (creatureData->size / 2);
	buf.data[3] = 255 * (creatureData->hue / 359.);
	buf.data[4] = 255 * (creatureData->preference / 1);
	buf.data[5] = 255 * (creatureData->metabolism / 2);

	Simulation::mutateBuffer(&buf, bodyMutation);

	creatureData->sight		 = (buf.data[0] * 2) / 255.;
	creatureData->speed		 = (buf.data[1] * 2) / 255.;
	creatureData->size		 = (buf.data[2] * 2) / 255.;
	creatureData->hue		 = (buf.data[3] * 359.) / 255.;
	creatureData->preference = (buf.data[4] / 255.);
	creatureData->metabolism = ((buf.data[5] * 2.) / 255.);

	if (!creatureData->useNEAT)
	{
		return;
	}

	for (int i = 0; i < networkCycles; i++)
	{
		int nonExistIndex = -1;

		in::Connection *connection = creatureData->connection;

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

			for (int x = (TOTAL_INPUT); x < TOTAL_INPUT + TOTAL_HIDDEN; x++)
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
			std::vector<int> hiddenNodes;
			hiddenNodes.reserve(creatureData->totalConnections);

			for (int i = 0; i < creatureData->totalConnections; i++)
			{
				if (!connection[i].exists)
				{
					continue;
				}

				if (connection[i].startNode < (TOTAL_INPUT + TOTAL_OUTPUT))
				{
					auto it = std::find(hiddenNodes.begin(), hiddenNodes.end(), connection[i].startNode);
					if (it != hiddenNodes.end())
					{
						hiddenNodes.emplace_back(connection[i].startNode);
					}
				}
			}

			int startNode = round((rand() / (float)RAND_MAX) * (TOTAL_INPUT + hiddenNodes.size() - 1));
			int endNode	  = round((rand() / (float)RAND_MAX) * (TOTAL_OUTPUT + hiddenNodes.size() - 1));

			if (startNode >= TOTAL_INPUT)
			{
				startNode -= TOTAL_INPUT;
				startNode = hiddenNodes[startNode];
			}

			if (endNode >= TOTAL_OUTPUT)
			{
				endNode -= TOTAL_OUTPUT;
				endNode = hiddenNodes[startNode];
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

template <typename T, typename U> void correctPosition(T &circle, U &otherCircle)
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
			circleOverlap  = (otherCircle.radius + circle.radius) - circleDistance;
		}

		agl::Vec<float, 2> offsetNormal = circleOffset.normalized();

		if (std::isnan(offsetNormal.x))
		{
			offsetNormal.x = 1;
			offsetNormal.y = 0;
		}

		agl::Vec<float, 2> pushback = offsetNormal * circleOverlap;

		float actingMass = circle.mass > otherCircle.mass ? otherCircle.mass : circle.mass;

		circle.posOffset -= pushback * (otherCircle.mass / (circle.mass + otherCircle.mass));
		otherCircle.posOffset += pushback * (circle.mass / (circle.mass + otherCircle.mass));

		circle.force -= pushback * actingMass;
		otherCircle.force += pushback * actingMass;
	}

#ifdef FOODPRESSURE
	else if constexpr (std::is_same_v<T, Food> && std::is_same_v<U, Food>)
	{
		if (circleDistance < 700)
		{
			float forceScalar = FOODPRESSURE / (circleDistance * circleDistance);

			agl::Vec<float, 2> force = circleOffset.normalized() * forceScalar;

			circle.force -= force;
			otherCircle.force += force;
		}
	}
#endif
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
	std::list<T> *Tlist = Tgrid->getList(gridPosition);
	std::list<U> *Ulist = Ugrid->getList(gridPosition);

	for (auto it = Tlist->begin(); it != Tlist->end(); it++)
	{
		T t = *it;

		for (auto it = Ulist->begin(); it != Ulist->begin(); it++)
		{
			U u = *it;

			correctPosition(*t, *u);
		}

		Ugrid->updateElements(gridPosition, {1, 1}, {1, 1}, [&](U u) { correctPosition(*t, *u); });
		Ugrid->updateElements(gridPosition, {1, 0}, {1, 0}, [&](U u) { correctPosition(*t, *u); });
		Ugrid->updateElements(gridPosition, {0, 1}, {0, 1}, [&](U u) { correctPosition(*t, *u); });
	}
}

template <typename T> void updatePhysics(agl::Vec<int, 2> gridPosition, Grid<T> *Tgrid)
{
	std::list<T> *list = Tgrid->getList(gridPosition);

	for (auto x = list->begin(); x != list->end(); x++)
	{
		T t1 = *x;

		for (auto y = std::next(x, 1); y != list->end(); y++)
		{
			T t2 = *y;

			correctPosition(*t1, *t2);
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

	env.view<Creature>([&](Creature &creature, auto) {
		creature.updateActions();

		creature.gridPosition = foodGrid->toGridPosition(creature.position, simulationRules.size);

		// if (std::isnan(creature->position.x))
		// {
		// 	existingCreatures->pop(i);
		// 	i--;
		// }

		creatureGrid->addData(creature.gridPosition, &creature);
	});

	foodGrid->clear();

	env.view<Food>([&](Food &food, auto) {
		PhysicsObj &circle = food;

#ifdef FOODDRAG
		float dragCoeficient = FOODDRAG;

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
#endif

#ifdef ACTIVEFOOD
		if ((food->nextPos - food->position).length() < 50)
		{
			food->nextRandPos(simulationRules.size);
		}

		food->force += (food->nextPos - food->position).normalized() / 100;
#endif

#ifdef FOODBORDER
		if (food.position.x < 0)
		{
			food.force.x += 1;
		}
		if (food.position.x > simulationRules.size.x)
		{
			food.force.x -= 1;
		}

		if (food.position.y < 0)
		{
			food.force.y += 1;
		}
		if (food.position.y > simulationRules.size.y)
		{
			food.force.y -= 1;
		}
#endif

		food.update();

		food.exists = true;

		foodGrid->addData(foodGrid->toGridPosition(food.position, simulationRules.size), &food);
	});

	meatGrid->clear();

	env.view<Meat>([&](Meat &meat, auto &it) {
		meat.lifetime--;

		if (meat.lifetime < 0)
		{
			auto next = std::next(it, -1);
			env.removeEntity<Meat>(it);
			it = next;
			return;
		}

		PhysicsObj &circle = meat;

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

		meat.update();

		meat.exists = true;

		meatGrid->addData(meatGrid->toGridPosition(meat.position, simulationRules.size), &meat);
	});

	env.view<Creature>([&](Creature &creature, auto &it) {
		// egg laying
		if (creature.layingEgg)
		{
			if (creature.energy > creature.eggTotalCost)
			{
				creature.incubating = true;
				creature.reward += 50;
			}
		}

		if (creature.incubating)
		{
			creature.energy -= PREGNANCY_COST;
			creature.eggDesposit += PREGNANCY_COST;

			if (creature.eggDesposit >= creature.eggTotalCost)
			{
				CreatureData creatureData = creature.creatureData;

				mutate(&creatureData, 50, 3);

				creatureData.startEnergy = creature.eggEnergyCost;

				this->addEgg(creatureData, creature.position);

				creature.incubating	 = false;
				creature.eggDesposit = 0;
			}
		}

		// creature eating
		if (creature.eating)
		{
			foodGrid->updateElements(
				foodGrid->toGridPosition(creature.position, simulationRules.size), {-1, -1}, {1, 1}, [&](Food *food) {
					if (!food->exists)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature.position - food->position;

					float angleDiff = offset.angle() - creature.rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature.radius + food->radius + EATRADIUS))
					{
						float energy =
							foodEnergyDensity * creature.preference * creature.preference * creature.preference;

						creature.energyDensity =
							newEnergyDensity(creature.biomass, creature.energyDensity, foodVol, energy);

						creature.biomass += foodVol;

						food->exists = false;
						creature.reward += energy * foodVol;
						this->removeFood(food);
					}
				});

			meatGrid->updateElements(
				meatGrid->toGridPosition(creature.position, simulationRules.size), {-1, -1}, {1, 1}, [&](Meat *meat) {
					if (!meat->exists)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature.position - meat->position;

					float angleDiff = offset.angle() - creature.rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature.radius + meat->radius + EATRADIUS))
					{
						float energy = (meatEnergyDensity * (1 - creature.preference));

						creature.energyDensity =
							newEnergyDensity(creature.biomass, creature.energyDensity, meat->energyVol, energy);

						creature.biomass += meat->energyVol;

						meat->exists = false;
						creature.reward += energy * meat->energyVol;
						this->removeMeat(meat);
					}
				});

			creatureGrid->updateElements(
				creatureGrid->toGridPosition(creature.position, simulationRules.size), {-1, -1}, {1, 1},
				[&](Creature *eatenCreature) {
					if (eatenCreature == &creature || eatenCreature->health < 0)
					{
						return;
					}

					agl::Vec<float, 2> offset = creature.position - eatenCreature->position;

					float angleDiff = offset.angle() - creature.rotation;

					if (abs(angleDiff - PI) > (PI / 4))
					{
						return;
					}

					if (offset.length() < (creature.radius + eatenCreature->radius + EATRADIUS))
					{
						// if (frame % BITEDELAY == 0)
						{
							float energy = (meatEnergyDensity * (1 - creature.preference));

							creature.energyDensity =
								newEnergyDensity(creature.biomass, creature.energyDensity, leachVol, energy);

							creature.biomass += leachVol;

							eatenCreature->health -= damage;
							creature.reward += energy * leachVol;
						}
					}
				});
		}

		// tired creature damage
		if (creature.energy <= 0)
		{
			creature.health--;
			creature.energy = 0;
		}

		// age damage
		if (creature.life < 0)
		{
			creature.health--;
		}

		// killing creature
		if (creature.health <= 0)
		{
			this->addMeat(creature.position, creature.maxHealth / 4);
			auto next = it;
			next--;
			this->removeCreature(it);
			it = next;
			return;
		}

		if (creature.energy > creature.maxEnergy)
		{
			creature.energy = creature.maxEnergy;
		}
		if (creature.biomass > creature.maxBiomass)
		{
			creature.biomass = creature.maxBiomass;
		}
	});

	env.view<Egg>([&](Egg &egg, auto &it) {
		egg.update();

		if (egg.timeleft <= 0)
		{
			Egg *hatchedEgg = &egg;

			CreatureData creatureData = hatchedEgg->creatureData;
			this->addCreature(creatureData, hatchedEgg->position);

			auto next = it;
			next--;
			removeEgg(it);
			it = next;

			return;
		}
	});

	// static int penalty = 0;
	//
	// int adjustedMaxFood =
	// 	int(simulationRules.maxFood * ((float)simulationRules.preferedCreatures
	// / existingCreatures->length)); adjustedMaxFood = std::min(adjustedMaxFood,
	// simulationRules.maxFood);
	//
	// if (existingCreatures->length > simulationRules.preferedCreatures)
	// {
	// 	penalty++;
	//
	// 	if (penalty > ((adjustedMaxFood + simulationRules.penaltyBuffer) *
	// simulationRules.penaltyPeriod))
	// 	{
	// 		penalty = ((adjustedMaxFood + simulationRules.penaltyBuffer) *
	// simulationRules.penaltyPeriod);
	// 	}
	// }
	// else
	// {
	// 	penalty--;
	//
	// 	if (penalty < 0)
	// 	{
	// 		penalty = 0;
	// 	}
	// }
	//
	// adjustedMaxFood -= (penalty / simulationRules.penaltyPeriod);

	// adding more food
	// int max = std::min(simulationRules.maxFood, adjustedMaxFood);
	// int max = 0;

	for (; env.getList<Food>().size() < foodCap;)
	{
		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addFood(position);
	}

	env.view<Creature>([&](auto &creature, auto) { creature.updateNetwork(foodGrid, creatureGrid, meatGrid); });
}

void Simulation::update()
{
	// this->threadableUpdate();
	this->updateSimulation();

	frame++;

#ifdef LOGCREATUREDATA
	int neat = 0;
	int rl	 = 0;
	int both = 0;

	env.view<Creature>([&](Creature &creature, auto it) {
		CreatureData *data = &creature.creatureData;

		if (data->useNEAT && !data->usePG)
		{
			neat++;
		}
		else if (!data->useNEAT && data->usePG)
		{
			rl++;
		}
		else if (data->useNEAT && data->usePG)
		{
			both++;
		}
	});

	totalNEAT.emplace_back(neat);
	totalRL.emplace_back(rl);
	totalBOTH.emplace_back(both);
#endif
}
