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

	env.setupGrid(simulationRules.size, simulationRules.gridResolution);

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

void Simulation::addCreature(CreatureData &creatureData, agl::Vec<float, 2> position)
{
	Creature &newCreature = env.addEntity<Creature>();
	newCreature.setup(creatureData, &simulationRules);
	newCreature.position = position;
	newCreature.rotation = ((float)rand() / (float)RAND_MAX) * PI * 2;
}

void Simulation::removeCreature(std::list<BaseEntity *>::iterator creature)
{
	env.removeEntity<Creature>(creature, [&](Creature &creature) { creature.clear(); });

	return;
}

void Simulation::addEgg(CreatureData &creatureData, agl::Vec<float, 2> position)
{
	Egg &newEgg = env.addEntity<Egg>();
	newEgg.setup(creatureData);
	newEgg.position = position;
}

void Simulation::removeEgg(std::list<BaseEntity *>::iterator egg)
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
	std::list<BaseEntity *>::iterator iterator;

	env.view<Food>([&](auto, auto it) {
		if (*it == (BaseEntity *)(DoNotUse *)food)
		{
			iterator = it;
		}
	});

	removeFood(iterator);
}

void Simulation::removeFood(std::list<BaseEntity *>::iterator food)
{
	env.removeEntity<Food>(food, [&](Food &food) {});

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

void Simulation::removeMeat(std::list<BaseEntity *>::iterator meat)
{
	env.removeEntity<Meat>(meat, [&](Meat &meat) {});

	return;
}

void Simulation::removeMeat(Meat *meat)
{
	std::list<BaseEntity *>::iterator iterator;

	env.view<Meat>([&](auto, auto it) {
		if (*it == (BaseEntity *)(DoNotUse *)meat)
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

	buf.mutate(bodyMutation);

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

void Simulation::updateSimulation()
{
	// adding more food

	for (; env.getList<Food>().size() < foodCap;)
	{
		agl::Vec<float, 2> position;
		position.x = (rand() / (float)RAND_MAX) * simulationRules.size.x;
		position.y = (rand() / (float)RAND_MAX) * simulationRules.size.y;

		this->addFood(position);
	}

	env.clearGrid();

	env.selfUpdate<Creature>([this](Creature &creature) {
		creature.updateNetwork();
		creature.updateActions();

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
			creature.exists = false;
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

		creature.updatePhysics();
	});

	env.selfUpdate<Food>([&](Food &food) {
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

		food.updatePhysics();
	});

	env.selfUpdate<Meat>([](Meat &meat) {
		meat.lifetime--;

		if (meat.lifetime < 0)
		{
			meat.exists = false;
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

		meat.updatePhysics();
	});

	env.selfUpdate<Egg>([&](Egg &egg) {
		egg.update();

		if (egg.timeleft <= 0)
		{
			Egg *hatchedEgg = &egg;

			CreatureData creatureData = hatchedEgg->creatureData;
			this->addCreature(creatureData, hatchedEgg->position);

			egg.exists = false;
			return;
		}
	});

	env.update<PhysicsObj, PhysicsObj, true>(
		[](PhysicsObj &circle, PhysicsObj &otherCircle, std::size_t hashT, std::size_t hashU) {
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
			else if (hashT == typeid(Food).hash_code() && hashU == typeid(Food).hash_code())
			{
				if (circleDistance < 700)
				{
					float forceScalar = FOODPRESSURE / (circleDistance * circleDistance);

					agl::Vec<float, 2> force = circleOffset.normalized() * forceScalar;

					circle.force -= force;
					otherCircle.force += force;
				}
			}
		}, [](PhysicsObj &circle){return circle.radius+50;});

	env.update<Creature, Creature>([env = &env](Creature &seeingCreature, Creature &creature, auto, auto) {
		agl::Vec<float, 2> offset	= seeingCreature.position - creature.position;
		float			   distance = offset.length();
			if(&seeingCreature == env->selected)
			{
			std::cout << "possible " << &creature << '\n';
			}

		if (distance > seeingCreature.creatureRelPos.distance)
		{
			return;
		}

		if (std::isnan(distance))
		{
			return;
		}

		seeingCreature.creatureRelPos.rotation = vectorAngle(offset) + seeingCreature.rotation;
		seeingCreature.creatureRelPos.distance = distance;

		seeingCreature.network->setInputNode(CREATURE_PREFERENCE, creature.preference);
	}, [](Creature &creature){return creature.creatureRelPos.distance;});

	env.update<Creature, Food>([](Creature &creature, Food &food, auto, auto) {
		agl::Vec<float, 2> offset	= creature.position - food.position;
		float			   distance = offset.length();

		if (distance > creature.foodRelPos.distance)
		{
			return;
		}

		if (std::isnan(distance))
		{
			return;
		}

		creature.foodRelPos.rotation = vectorAngle(offset) + creature.rotation;
		creature.foodRelPos.distance = distance;
	}, [](Creature &creature){return creature.foodRelPos.distance;});

	env.update<Creature, Meat>([&](Creature &creature, Meat &meat, auto, auto) {
		agl::Vec<float, 2> offset	= creature.position - meat.position;
		float			   distance = offset.length();

		if (distance > creature.meatRelPos.distance)
		{
			return;
		}

		if (std::isnan(distance))
		{
			return;
		}

		creature.meatRelPos.rotation = vectorAngle(offset) + creature.rotation;
		creature.meatRelPos.distance = distance;
	}, [](Creature &creature){return creature.meatRelPos.distance;});

	// creature eating
	env.update<Creature, Food>([&](Creature &creature, Food &food, auto, auto) {
		if (!creature.eating)
		{
			return;
		}

		agl::Vec<float, 2> offset = creature.position - food.position;

		float angleDiff = offset.angle() - creature.rotation;

		if (abs(angleDiff - PI) > (PI / 4))
		{
			return;
		}

		if (offset.length() < (creature.radius + food.radius + EATRADIUS))
		{
			float energy = foodEnergyDensity * creature.preference * creature.preference * creature.preference;

			creature.energyDensity = newEnergyDensity(creature.biomass, creature.energyDensity, foodVol, energy);

			creature.biomass += foodVol;

			food.exists = false;
			creature.reward += energy * foodVol;
		}
	}, [](Creature &creature){return creature.radius + EATRADIUS + 50;});

	env.update<Creature, Meat>([&](Creature &creature, Meat &meat, auto, auto) {
		if (!creature.eating)
		{
			return;
		}

		agl::Vec<float, 2> offset = creature.position - meat.position;

		float angleDiff = offset.angle() - creature.rotation;

		if (abs(angleDiff - PI) > (PI / 4))
		{
			return;
		}

		if (offset.length() < (creature.radius + meat.radius + EATRADIUS))
		{
			float energy = (meatEnergyDensity * (1 - creature.preference));

			creature.energyDensity = newEnergyDensity(creature.biomass, creature.energyDensity, meat.energyVol, energy);

			creature.biomass += meat.energyVol;

			meat.exists = false;
			creature.reward += energy * meat.energyVol;
		}
	}, [](Creature &creature){return creature.radius + EATRADIUS + 50;});

	env.update<Creature, Creature>([&](Creature &creature, Creature &eatenCreature, auto, auto) {
		if (eatenCreature.health < 0 || !creature.eating)
		{
			return;
		}

		agl::Vec<float, 2> offset = creature.position - eatenCreature.position;

		float angleDiff = offset.angle() - creature.rotation;

		if (abs(angleDiff - PI) > (PI / 4))
		{
			return;
		}

		if (offset.length() < (creature.radius + eatenCreature.radius + EATRADIUS))
		{
			// if (frame % BITEDELAY == 0)
			{
				float energy = (meatEnergyDensity * (1 - creature.preference));

				creature.energyDensity = newEnergyDensity(creature.biomass, creature.energyDensity, leachVol, energy);

				creature.biomass += leachVol;

				eatenCreature.health -= damage;
				creature.reward += energy * leachVol;
			}
		}
	}, [](Creature &creature){return creature.radius + EATRADIUS + 50;});

	while (env.pool.active())
	{
	}
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
