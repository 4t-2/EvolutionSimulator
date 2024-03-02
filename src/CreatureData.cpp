#include "../inc/CreatureData.hpp"

CreatureData::CreatureData()
{
	netStr = nullptr;

	return;
}

CreatureData::CreatureData(float sight, int hue, std::vector<SegmentData> &segs, std::vector<in::Connection> &cons, int maxCon)
{
	this->sight = sight;
	this->hue	= hue;

	// default design

	sd = segs;

	std::vector<in::Connection> vec = cons;

	while (vec.size() < maxCon)
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

	netStr = new in::NetworkStructure(maxCon, totalSegJoints(sd) * 2 + 2, 0, totalSegJoints(sd), vec);

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
