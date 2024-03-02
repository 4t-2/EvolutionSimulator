#include "../inc/CreatureData.hpp"

CreatureData::CreatureData()
{
	netStr = nullptr;

	return;
}

CreatureData::CreatureData(float sight, int hue, std::vector<SegmentData> &segs)
{
	this->sight = sight;
	this->hue	= hue;

	// default design

	// sd = segs;

	sd = {
		{{24, 24}, {}},
		{{4, 24}, {}},
	};

	// int length = 2 + rand() % (5 - 2 + 1);
	//
	// agl::Vec<int, 2> lastVec = {100, 100};
	//
	// for (int i = 0; i < length; i++)
	// {
	// 	agl::Vec<int, 2> size = {4 + rand() % (24 - 4 + 1), 4 + rand() % (24 - 4
	// + 1)};
	//
	// 	size.x = size.x > lastVec.x ? lastVec.x : size.x;
	// 	lastVec.x = size.x;
	//
	// 	int limbs = -1 + rand() % (1 - (-1) + 1);
	//
	// 	sd.push_back({size});
	//
	// 	for (int x = 0; x < limbs; x++)
	// 	{
	// 		agl::Vec<int, 2> sizeLimb = {4 + rand() % (12 - 4 + 1), 4 +
	// rand() % (24 - 4 + 1)};
	//
	// 		sizeLimb.x = sizeLimb.x > size.y ? size.y : sizeLimb.x;
	//
	// 		sd.back().branch.push_back({sizeLimb});
	// 	}
	// }

	// con
	// sin

	// ang
	// mot
	// ang
	// mot

	// mov
	// mov

	std::vector<in::Connection> vec = {in::Connection{1, 4, 1}};

	while (vec.size() < 10)
	{
		in::Connection c;
		c.id		= -1;
		c.valid		= false;
		c.exists	= false;
		c.weight	= -1;
		c.startNode = -1;
		c.endNode	= -1;
		vec.push_back(c);
	}

	netStr = new in::NetworkStructure(10, totalSegJoints(sd) * 2 + 2, 0, totalSegJoints(sd), vec);

	// std::cout << *netStr << '\n';

	// recurse(Output(std::cout), sd, "name");

	// netStr = new in::NetworkStructure(totalSegJoints(sd) * 2 + 2, {},
	// totalSegJoints(sd), false); in::NetworkStructure::randomWeights(*netStr);

	// (in::NetworkStructureType &)netStr->type = in::Dynamic;

	return;
}

CreatureData::CreatureData(const CreatureData &creatureData)
{
	delete netStr;
	netStr = new in::NetworkStructure(*creatureData.netStr);

	sight		= creatureData.sight;
	hue			= creatureData.hue;
	startEnergy = creatureData.startEnergy;
	preference	= creatureData.preference;
	metabolism	= creatureData.metabolism;
	useNEAT		= creatureData.useNEAT;
	usePG		= creatureData.usePG;
	sd			= creatureData.sd;
}

void CreatureData::operator=(CreatureData &creatureData)
{
	delete netStr;
	netStr = new in::NetworkStructure(*creatureData.netStr);

	sight		= creatureData.sight;
	hue			= creatureData.hue;
	startEnergy = creatureData.startEnergy;
	preference	= creatureData.preference;
	metabolism	= creatureData.metabolism;
	useNEAT		= creatureData.useNEAT;
	usePG		= creatureData.usePG;
	sd			= creatureData.sd;
}

CreatureData::~CreatureData()
{
	delete netStr;

	return;
}

int CreatureData::totalSegJoints(std::vector<SegmentData> &segs)
{
	int total = 0;

	for (int i = 0; i < segs.size(); i++)
	{
		if (i != 0)
		{
			total++;
		}

		total += segs[i].branch.size() * 2;
	}

	return total;
}
