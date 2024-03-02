#include "../inc/Simulation.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <random>
#include <string>
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

int generateRandomNumber(int min, int max)
{
	int ran = rand();
	std::cout << ran << '\n';
	std::cout << min << '\n';
	std::cout << max << '\n';
	std::cout << '\n';
	return min + ran % (max - min + 1);
}

void Simulation::create(SimulationRules simulationRules, int seed)
{
	active = true;
	frame  = 0;

	this->simulationRules = simulationRules;

	srand(seed);

	env.setupGrid(simulationRules.size, simulationRules.gridResolution);

	for (int i = 0; i < simulationRules.startingCreatures; i++)
	{
		CreatureData creatureData(1, generateRandomNumber(0, 255), simulationRules.startBody);

		creatureData.useNEAT	= true;
		creatureData.usePG		= false;
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

	env.setThreads(simulationRules.threads);

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
	newCreature.setup(creatureData, &simulationRules, env, position);
	// // newCreature.rotation = ((float)rand() / (float)RAND_MAX) * PI * 2;
	//
	// newCreature.segments.emplace_back(&newCreature);
	//
	// auto &r0 = env.addEntity<TestObj>();
	// {
	// 	r0.setup({position.x, position.y + newCreature.size.y}, {4,
	// newCreature.size.y}, 1);
	//
	// 	PhysicsObj::addJoint(r0, {0, -r0.size.y / 2}, newCreature, {0,
	// newCreature.size.y / 2}); 	r0.motor = .1;
	//
	// 	newCreature.segments.emplace_back(&r0);
	// }
	//
	// {
	// 	auto &r1 = env.addEntity<TestObj>();
	// 	{
	// 		r1.setup(newCreature.position + agl::Vec<float,
	// 2>{newCreature.size.x
	// / 2 + 8, 0}, {2, 16}, 1); 		r1.rotation = -PI / 2;
	//
	// 		PhysicsObj::addJoint(r1, {0, -r1.size.y / 2}, newCreature,
	// {newCreature.size.x / 2, 0});
	//
	// 		newCreature.segments.emplace_back(&r1);
	// 	}
	// }
	// {
	// 	auto &r1 = env.addEntity<TestObj>();
	// 	{
	// 		r1.setup(newCreature.position + agl::Vec<float,
	// 2>{-newCreature.size.x / 2 - 8, 0}, {2, 16}, 1); 		r1.rotation = PI
	// / 2;
	//
	// 		PhysicsObj::addJoint(r1, {0, -r1.size.y / 2}, newCreature,
	// {-newCreature.size.x / 2, 0});
	//
	// 		newCreature.segments.emplace_back(&r1);
	// 	}
	// }
	//
	// // newCreature.position.x += newCreature.size.x / 2;
	// // newCreature.position.y += newCreature.size.y / 2;
	// // newCreature.rotation = PI / 2;
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
	Meat &newMeat	 = env.addEntity<Meat>();
	newMeat.position = position;
	// newMeat.radius	  = (energy / 50.) * 5;
	newMeat.rotation = ((float)rand() / (float)RAND_MAX) * 360;
	newMeat.lifetime = newMeat.energyVol * 288;
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

void recurseSegs(std::vector<SegmentData> &sd)
{
	for (SegmentData &s : sd)
	{
		if (generateRandomNumber(0, 1))
		{
			agl::Vec<int, 2> delta = {generateRandomNumber(-1, 1), generateRandomNumber(-1, 1)};

			s.size += delta;
		}

		if (s.size.x < 1)
		{
			s.size.x = 1;
		}
		else if (s.size.y < 1)
		{
			s.size.y = 1;
		}

		recurseSegs(s.branch);
	}
}

void mutateBodu(in::NetworkStructure *netStr, std::vector<SegmentData> &sd)
{
	if (1) // remove
	{
		int seg = 1;

		int del = 1 + CreatureData::totalSegJoints(sd[seg].branch);

		sd.erase(std::next(sd.begin(), seg));

		int node = 2 + (2 * seg) + netStr->totalInputNodes + netStr->totalHiddenNodes;

		for (int i = node; i < (node + del); i++)
		{
			for (int x = 0; x < netStr->totalConnections; x++)
			{
				if (netStr->connection[x].endNode == node || netStr->connection[x].startNode == node)
				{
					netStr->removeConnection(x);
					continue;
				}

				if (netStr->connection[x].endNode >= (node + del))
				{
					((in::Connection *)&netStr->connection[x])->endNode -= del;
				}
				else if (netStr->connection[x].startNode >= (node + del))
				{
					((in::Connection *)&netStr->connection[x])->endNode -= del;
				}
			}
		}
	}
	else if (0) // add
	{
		int seg = 0;

		sd.insert(std::next(sd.begin(), seg), {sd[seg].size});
	}
	else if (0) // duplicate
	{
		int seg = 0;

		sd.insert(std::next(sd.begin(), seg), sd[seg]);
		Debug::log.emplace_back(std::to_string(sd.size()) + " is new");
	}
}

bool inRange(int num, int min, int max)
{
	return num >= min && num <= max ? true : false;
}

int segTotalOneSided(std::vector<SegmentData> &segs)
{
	int total = 0;

	for (int i = 0; i < segs.size(); i++)
	{
		if (i != 0)
		{
			total++;
		}

		total += segs[i].branch.size();
	}

	return total;
}

void mutate(CreatureData *creatureData, int bodyMutation, int networkCycles)
{
	creatureData->hue = mutShift(creatureData->hue / 60., 0, 359. / 60) * 60;

	{
		int perc = generateRandomNumber(0, 4);

		if (perc == 0)
		{
			int choice = generateRandomNumber(0, 1);

			std::vector<int> jointMap;

			for (int i = 0; i < creatureData->netStr->totalNodes; i++)
			{
				jointMap.push_back(i);
			}

			if (choice == 0 && creatureData->sd.size() > 1) // remove
			{
				int segTot		   = CreatureData::totalSegJoints(creatureData->sd);
				int segTotOneSided = segTotalOneSided(creatureData->sd);

				int seg = generateRandomNumber(1, segTotOneSided - 1);

				int del = 0;

				int segTrack   = 0;
				int actualNode = -1;

				for (int i = 0; i < creatureData->sd.size(); i++)
				{
					if (seg == segTrack)
					{
						del = creatureData->sd[i].branch.size() + 1;
						creatureData->sd.erase(std::next(creatureData->sd.begin(), i));
						goto exit1;
					}
					segTrack++;
					actualNode++;
					for (int x = 0; x < creatureData->sd[i].branch.size(); x++)
					{
						if (seg == segTrack)
						{
							del = 1;
							creatureData->sd[i].branch.erase(std::next(creatureData->sd[i].branch.begin(), x));
							goto exit1;
						}
						segTrack++;
						actualNode += 2;
					}
				}

			exit1:;

				for (auto it = jointMap.begin(); it != jointMap.end();)
				{
					int node = *it;

					if (node == (actualNode * 2) + 2)
					{
						it = jointMap.erase(it, std::next(it, del * 2));

						continue;
					}
					if (node == creatureData->netStr->totalInputNodes + actualNode)
					{
						it = jointMap.erase(it, std::next(it, del));
						continue;
					}

					it++;
				}

				for (int i = 0; i < creatureData->netStr->totalConnections; i++)
				{
					in::Connection *con = (in::Connection *)&creatureData->netStr->connection[i];

					if (!con->exists || !con->valid)
					{
						continue;
					}

					if (inRange(con->startNode, (actualNode * 2) + 2, (actualNode * 2) + 2 + (del * 2) - 1))
					{
						creatureData->netStr->removeConnection(i);
					}
					if (inRange(con->startNode, creatureData->netStr->totalInputNodes + actualNode,
								creatureData->netStr->totalInputNodes + actualNode + del - 1))
					{
						creatureData->netStr->removeConnection(i);
					}
					if (inRange(con->endNode, creatureData->netStr->totalInputNodes + actualNode,
								creatureData->netStr->totalInputNodes + actualNode + del - 1))
					{
						creatureData->netStr->removeConnection(i);
					}
				}
			}
			else if (choice == 1) // extend
			{
				int segTot		   = CreatureData::totalSegJoints(creatureData->sd);
				int segTotOneSided = segTotalOneSided(creatureData->sd);

				int seg = generateRandomNumber(0, creatureData->sd.size() - 1);

				enum ExTy
				{
					Length,
					Split
				} et;

				int actualNode = 0;

				for (int i = 0; i < creatureData->sd.size(); i++)
				{
					if (i == seg)
					{
						if (i == (creatureData->sd.size() - 1))
						{
							creatureData->sd.push_back({creatureData->sd[i].size / 2});
							et = Length;
						}
						else
						{
							agl::Vec<int, 2> size;
							if (creatureData->sd[i].branch.size() == 0)
							{
								size = creatureData->sd[i].size / 2;
							}
							else
							{
								size = creatureData->sd[i].branch.back().size / 2;
							}

							creatureData->sd[i].branch.push_back({size});
							et = Split;
						}
						break;
					}

					actualNode += 1 + creatureData->sd[i].branch.size();
				}

				for (auto it = jointMap.begin(); it != jointMap.end();)
				{
					int node = *it;

					if (node == (actualNode * 2) + 2 - 1)
					{
						it = jointMap.insert(std::next(it, 1), -69);
						it = jointMap.insert(std::next(it, 1), -69);

						if (et == Split)
						{
							it = jointMap.insert(std::next(it, 1), -69);
							it = jointMap.insert(std::next(it, 1), -69);
						}

						it = std::next(it, 1);

						continue;
					}
					if (node == creatureData->netStr->totalInputNodes + actualNode - 1)
					{
						it = jointMap.insert(std::next(it, 1), -69);

						if (et == Split)
						{
							it = jointMap.insert(std::next(it, 1), -69);
						}

						it = std::next(it, 1);
						continue;
					}

					it++;
				}
			}

			for (int i = 0; i < creatureData->netStr->totalConnections; i++)
			{
				in::Connection *con = (in::Connection *)&creatureData->netStr->connection[i];
				for (int i = 0; i < jointMap.size(); i++)
				{
					if (jointMap[i] == con->startNode)
					{
						con->startNode = i;
					}
					if (jointMap[i] == con->endNode)
					{
						con->endNode = i;
					}
				}
			}

			(int &)creatureData->netStr->totalInputNodes  = CreatureData::totalSegJoints(creatureData->sd) * 2 + 2;
			(int &)creatureData->netStr->totalOutputNodes = CreatureData::totalSegJoints(creatureData->sd);
			(int &)creatureData->netStr->totalNodes =
				(int &)creatureData->netStr->totalInputNodes + (int &)creatureData->netStr->totalOutputNodes;
		}
	}

	recurseSegs(creatureData->sd);

	if (!creatureData->useNEAT)
	{
		return;
	}

	for (int i = 0; i < networkCycles; i++)
	{
		int nonExistIndex = -1;

		in::Connection *connection = (in::Connection *)creatureData->netStr->connection;

		for (int i = 0; i < creatureData->netStr->totalConnections; i++)
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
			int index = round((rand() / (float)RAND_MAX) * (creatureData->netStr->totalConnections - 1));
			int start = connection[index].startNode;
			int end	  = connection[index].endNode;

			connection[index].weight = ((rand() / (float)RAND_MAX) * 4) - 2;
		}
		else if (type == 1)
		{
			int index = round((rand() / (float)RAND_MAX) * (creatureData->netStr->totalConnections - 1));

			creatureData->netStr->removeConnection(index);
		}
		else if (type == 2)
		{
			int node = -1;

			for (int x = (creatureData->netStr->totalInputNodes);
				 x < creatureData->netStr->totalInputNodes + creatureData->netStr->totalHiddenNodes; x++)
			{
				node = x;

				for (int i = 0; i < creatureData->netStr->totalConnections; i++)
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
				int index = round((rand() / (float)RAND_MAX) * (creatureData->netStr->totalConnections - 1));

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
			hiddenNodes.reserve(creatureData->netStr->totalConnections);

			for (int i = 0; i < creatureData->netStr->totalConnections; i++)
			{
				if (!connection[i].exists)
				{
					continue;
				}

				if (connection[i].startNode <
					(creatureData->netStr->totalInputNodes + creatureData->netStr->totalOutputNodes))
				{
					auto it = std::find(hiddenNodes.begin(), hiddenNodes.end(), connection[i].startNode);
					if (it != hiddenNodes.end())
					{
						hiddenNodes.emplace_back(connection[i].startNode);
					}
				}
			}

			int startNode =
				round((rand() / (float)RAND_MAX) * (creatureData->netStr->totalInputNodes + hiddenNodes.size() - 1));
			int endNode =
				round((rand() / (float)RAND_MAX) * (creatureData->netStr->totalOutputNodes + hiddenNodes.size() - 1));

			if (startNode >= creatureData->netStr->totalInputNodes)
			{
				startNode -= creatureData->netStr->totalInputNodes;
				startNode = hiddenNodes[startNode];
			}

			if (endNode >= creatureData->netStr->totalOutputNodes)
			{
				endNode -= creatureData->netStr->totalOutputNodes;
				endNode = hiddenNodes[startNode];
			}
			else
			{
				endNode += creatureData->netStr->totalInputNodes;
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

void oldAirRes(PhysicsObj &o)
{
	float velAng = o.velocity.angle();

	if (!std::isnan(velAng))
	{
		float			   velMag = o.velocity.length();
		agl::Vec<float, 2> velNor = o.velocity.normalized();

		float relAng = velAng - o.GetAngle();

		float side1 = abs(o.size.x * cos(relAng));
		float side2 = abs(o.size.y * sin(relAng));

		// agl::Vec<float, 2> norm = {-cos(relAng), -sin(relAng)};

		constexpr float density = .04;

		agl::Vec<float, 2> drag1 = velNor * -(velMag * velMag * density * side1);
		// drag1					 = {drag1.x * norm.x, drag1.y *
		// norm.y}; norm					 = perp(norm);
		agl::Vec<float, 2> drag2 = velNor * -(velMag * velMag * density * side2);
		// drag2					 = {drag2.x * norm.x, drag2.y *
		// norm.y};

		o.ApplyForceToCenter(drag1 + drag2);
	}
}

void calcAirResForSide(PhysicsObj &o, agl::Vec<float, 2> norm, std::function<float(float)> trigFunc, float relAng,
					   float side1, float side2, float velMag)
{
	constexpr float density = .04;

	agl::Vec<float, 2> size	   = {abs(side1 * trigFunc(relAng)), velMag * velMag};
	float			   mass	   = size.x * size.y * density;
	float			   inertia = (1 / 12.) * mass * (size.x * size.x + size.y * size.y);

	agl::Vec<float, 2> airAccLin;
	float			   airAccRot;

	agl::Vec<float, 2> r1 = norm * side2 / 2;

	World::resolve(o.velocity, {0, 0}, o.angularVelocity, 0, o.invInertia, 1 / inertia, o.invMass, 1 / mass, norm, r1,
				   norm * velMag / -2, o.acceleration, airAccLin, o.angularAcceleration, airAccRot, 1);
}

void newAirRes(PhysicsObj &o)
{
	float velAng = o.velocity.angle();

	agl::Mat4f rot;
	rot.rotateZ(-o.radToDeg());

	agl::Vec<float, 2> norms[4];
	norms[0] = rot * agl::Vec<float, 2>{0, -1};
	norms[1] = rot * agl::Vec<float, 2>{1, 0};
	norms[2] = rot * agl::Vec<float, 2>{0, 1};
	norms[3] = rot * agl::Vec<float, 2>{-1, 0};

	float			   velMag = o.velocity.length();
	agl::Vec<float, 2> velNor = o.velocity.normalized();

	float relAng = velAng - o.GetAngle();

	for (int i = 0; i < 4; i++)
	{
		auto norm = norms[i];
		if (norm.dot(o.velocity) > 0)
		{
			calcAirResForSide(o, norm, i % 2 ? sinf : cosf, relAng, i % 2 ? o.size.y : o.size.x,
							  i % 2 ? o.size.x : o.size.y, velMag);
		}
	}
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

	env.view<PhysicsObj>([](PhysicsObj &o, auto it) {
		newAirRes(o);

		o.updatePhysics();
	});
	env.clearGrid();

	env.selfUpdate<Creature>([this](Creature &creature) {
		creature.updateNetwork();
		creature.updateActions();

		// std::cout << creature.position << '\n';

		// egg laying
		if (creature.layingEgg)
		{
			if (creature.energy > creature.eggTotalCost)
			{
				creature.incubating = true;
				creature.reward += 50;
			}
		}

		int iBio = (int)creature.biomass;
		for (int i = 0; i < iBio; i++)
		{
			CreatureData creatureData = creature.creatureData;

			mutate(&creatureData, simulationRules.bodyMutation, simulationRules.brainMutation);

			creatureData.startEnergy = creature.eggEnergyCost;

			agl::Vec<float, 2> pos;
			pos.x = simulationRules.size.x * ((float)rand() / (float)RAND_MAX);
			pos.y = simulationRules.size.y * ((float)rand() / (float)RAND_MAX);

			this->addEgg(creatureData, pos);

			creature.incubating	 = false;
			creature.eggDesposit = 0;
		}

		creature.biomass = 0;

		// tired creature damage
		if (creature.energy <= 0)
		{
			// creature.health--;
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
			// this->addMeat(creature.position, creature.maxHealth / 4);
			creature.exists = false;
			return;
		}

		if (creature.velocity.length() > 10)
		{
			creature.exists = false;
			return;
		}

		if (creature.energy > creature.maxEnergy)
		{
			creature.energy = creature.maxEnergy;
		}

		for (auto itx = creature.segments.begin(); itx != creature.segments.end(); itx++)
		{
			for (auto ity = std::next(itx, 1); ity != creature.segments.end(); ity++)
			{
				PhysicsObj &b1 = **itx;
				PhysicsObj &b2 = **ity;

				if (b1.rootConnect == &b2)
				{
					World::motor(b1);
				}
				else if (b2.rootConnect == &b1)
				{
					World::motor(b2);
				}
			}
		}
	});

	// env.view<PhysicsObj>([](auto &o, auto it)
	// {
	// 		o.velocity += o.acceleration;
	// 		o.acceleration = {0, 0};
	// 		circle.angularVelocity += circle.angularAcceleration;
	// 		circle.angularAcceleration = 0;
	// });

	env.selfUpdate<Food>([&](Food &food) {
		PhysicsObj &circle = food;

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
	});

	env.selfUpdate<Meat>([](Meat &meat) {
		meat.lifetime--;

		if (meat.lifetime < 0)
		{
			meat.exists = false;
			return;
		}
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

	env.selfUpdate<TestObj>([](auto &o) {});

	while (env.pool.active())
	{
	}

	env.update<PhysicsObj, PhysicsObj, true>(
		[](PhysicsObj &circle, PhysicsObj &otherCircle, std::size_t hashT, std::size_t hashU) {
			std::vector<ConstraintFailure> failure;

			if (circle.rootConnect == &otherCircle)
			{
				JointConstraint::probe(circle, otherCircle, failure);
			}
			else if (otherCircle.rootConnect == &circle)
			{
				JointConstraint::probe(otherCircle, circle, failure);
			}
			else
			{
				CollisionConstraint::probe(circle, otherCircle, failure);
			}

			for (ConstraintFailure &f : failure)
			{
				World::resolve(f, 1);
			}
		},
		[](PhysicsObj &circle) { return 100; });

	while (env.pool.active())
	{
	}

	env.view<PhysicsObj>([](PhysicsObj &o, auto) {
		if (o.collideCount)
		{
			o.acceleration /= o.collideCount;
			o.angularAcceleration /= o.collideCount;
			o.posOffset /= o.collideCount;
			o.rotOffset /= o.collideCount;

			o.collideCount = 0;
		}
	});

	env.update<Creature, Food>(
		[](Creature &creature, Food &food, auto, auto) {
			for (auto &seg : creature.segments)
			{
				if ((seg->position - food.position).length() < 20)
				{
					creature.biomass += 1;
					food.exists = false;
				}
			}
		},
		[](Creature &creature) { return creature.foodRelPos.distance; });

	// creature eating
	// env.update<Creature, Food>(
	// 	[&](Creature &creature, Food &food, auto, auto) {
	// 		if (!creature.eating)
	// 		{
	// 			return;
	// 		}
	//
	// 		agl::Vec<float, 2> offset = creature.position - food.position;
	//
	// 		float angleDiff = offset.angle() - creature.rotation;
	//
	// 		if (abs(angleDiff - PI) > (PI / 4))
	// 		{
	// 			return;
	// 		}
	//
	// 		if (offset.length() < (creature.radius + food.radius +
	// EATRADIUS))
	// 		{
	// 			float energy = foodEnergyDensity * creature.preference *
	// creature.preference * creature.preference;
	//
	// 			creature.energyDensity =
	// newEnergyDensity(creature.biomass, creature.energyDensity, foodVol,
	// energy);
	//
	// 			creature.biomass += foodVol;
	//
	// 			food.exists = false;
	// 			creature.reward += energy * foodVol;
	// 		}
	// 	},
	// 	[](Creature &creature) { return creature.radius + EATRADIUS + 50; });
	//
	// env.update<Creature, Meat>(
	// 	[&](Creature &creature, Meat &meat, auto, auto) {
	// 		if (!creature.eating)
	// 		{
	// 			return;
	// 		}
	//
	// 		agl::Vec<float, 2> offset = creature.position - meat.position;
	//
	// 		float angleDiff = offset.angle() - creature.rotation;
	//
	// 		if (abs(angleDiff - PI) > (PI / 4))
	// 		{
	// 			return;
	// 		}
	//
	// 		if (offset.length() < (creature.radius + meat.radius +
	// EATRADIUS))
	// 		{
	// 			float energy = (meatEnergyDensity * (1 -
	// creature.preference));
	//
	// 			creature.energyDensity =
	// 				newEnergyDensity(creature.biomass,
	// creature.energyDensity, meat.energyVol, energy);
	//
	// 			creature.biomass += meat.energyVol;
	//
	// 			meat.exists = false;
	// 			creature.reward += energy * meat.energyVol;
	// 		}
	// 	},
	// 	[](Creature &creature) { return creature.radius + EATRADIUS + 50; });
	//
	// env.update<Creature, Creature>(
	// 	[&](Creature &creature, Creature &eatenCreature, auto, auto) {
	// 		if (eatenCreature.health < 0 || !creature.eating)
	// 		{
	// 			return;
	// 		}
	//
	// 		agl::Vec<float, 2> offset = creature.position -
	// eatenCreature.position;
	//
	// 		float angleDiff = offset.angle() - creature.rotation;
	//
	// 		if (abs(angleDiff - PI) > (PI / 4))
	// 		{
	// 			return;
	// 		}
	//
	// 		if (offset.length() < (creature.radius + eatenCreature.radius +
	// EATRADIUS))
	// 		{
	// 			// if (frame % BITEDELAY == 0)
	// 			{
	// 				float energy = (meatEnergyDensity * (1 -
	// creature.preference));
	//
	// 				creature.energyDensity =
	// 					newEnergyDensity(creature.biomass,
	// creature.energyDensity, leachVol, energy);
	//
	// 				creature.biomass += leachVol;
	//
	// 				eatenCreature.health -= damage;
	// 				creature.reward += energy * leachVol;
	// 			}
	// 		}
	// 	},
	// 	[](Creature &creature) { return creature.radius + EATRADIUS + 50; });

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
