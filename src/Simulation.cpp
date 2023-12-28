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

	{
		auto &a	   = env.addEntity<PhyCircle>();
		a.position = {0, 0};
	}
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

#ifdef ACTIVEFOOD
	newFood.nextRandPos(simulationRules.size);
#endif
}

void Simulation::removeFood(std::list<BaseEntity *>::iterator food)
{
	return;
}

void Simulation::addMeat(agl::Vec<float, 2> position, float energy)
{
}

void Simulation::addMeat(agl::Vec<float, 2> position)
{
	this->addMeat(position, 50);
}

void Simulation::removeMeat(std::list<BaseEntity *>::iterator meat)
{
	return;
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

	env.clearGrid();

	env.selfUpdate<PhyCircle>([](PhyCircle &o) { o.updatePhysics(); });
	env.selfUpdate<PhySquare>([](PhySquare &o) { o.updatePhysics(); });

	env.update<PhyCircle, PhyCircle, true>(
		[](PhyCircle &circle, PhyCircle &otherCircle, std::size_t hashT, std::size_t hashU) {
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
		},
		[](PhyCircle &circle) { return circle.radius + 50; });

	env.update<PhyCircle, PhySquare, true>(
		[](PhyCircle &circle, PhySquare &square, std::size_t hashT, std::size_t hashU) {
			agl::Vec<float, 2> interOffset;

			{
				agl::Vec<float, 2> circleOffset = square.position - circle.position;

				float circleDistance = circleOffset.length();

				float angleDelta = circleOffset.angle() + square.rotation;
				float distance;

				angleDelta = abs(angleDelta);

				interOffset.x = -std::sin(angleDelta) * circleDistance;
				interOffset.y = std::cos(angleDelta) * circleDistance;

				if (interOffset.x > (square.length / 2))
				{
					interOffset.x = square.length / 2;
				}
				else if (interOffset.x < (-square.length / 2))
				{
					interOffset.x = -square.length / 2;
				}
				if (interOffset.y > (square.length / 2))
				{
					interOffset.y = square.length / 2;
				}
				else if (interOffset.y < (-square.length / 2))
				{
					interOffset.y = -square.length / 2;
				}
			}

			agl::Mat4f rot;
			rot.rotateZ(square.radToDeg());

			interOffset = rot * interOffset;

			agl::Vec<float, 2> squarePointVec = interOffset + square.position;
			agl::Vec<float, 2> offset		  = squarePointVec - circle.position;
			float			   distance		  = offset.length();

			float overlap = circle.radius - distance;

#define DEBUGLOG(x) std::cout << #x << " = " << x << '\n';

			std::cout << "\n";
			DEBUGLOG(overlap);
			DEBUGLOG(interOffset);
			DEBUGLOG(distance);

			// return 0;

			if (overlap > 0)
			{
				agl::Vec<float, 2> offsetNormal = offset.normalized();
				agl::Vec<float, 2> test =
					agl::Vec<float, 2>{offsetNormal.y * interOffset.x, offsetNormal.x * interOffset.y};

				DEBUGLOG(offsetNormal);
				DEBUGLOG(test);

				if (std::isnan(offsetNormal.x))
				{
					offsetNormal.x = 1;
					offsetNormal.y = 0;
				}

				float restitution = 1;

				auto relVel = square.velocity - circle.velocity;

				float impulse = (relVel * -(1 + restitution)).dot(offsetNormal) //
								/												//
								offsetNormal.dot(offsetNormal * ((1 / square.mass) + (1 / circle.mass)));

                square.velocity += offsetNormal * (impulse / square.mass);
                circle.velocity -= offsetNormal * (impulse / circle.mass);

				// agl::Vec<float, 2> pushback = offsetNormal * overlap;
				//
				// float actingMass = circle.mass > square.mass ? square.mass :
				// circle.mass;
				//
				// circle.posOffset -= pushback * (square.mass / (circle.mass +
				// square.mass)); square.posOffset += pushback * (circle.mass /
				// (circle.mass + square.mass));
				//
				// circle.force -= pushback * actingMass;
				// square.force += pushback * actingMass;
				//
				// float force = (pushback * actingMass).length();
				//
				// square.angularVelocity += (2 * force) / (actingMass * test.x);
				// square.angularVelocity += (2 * force) / (actingMass * test.y);
			}
		},
		[](PhyCircle &circle) { return circle.radius + 50; });

	while (env.pool.active())
	{
	}
}

void Simulation::update()
{
	// this->threadableUpdate();
	this->updateSimulation();

	frame++;
}
