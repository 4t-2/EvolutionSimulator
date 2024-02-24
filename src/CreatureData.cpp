#include "../inc/CreatureData.hpp"

CreatureData::CreatureData()
{
	netStr = nullptr;

	return;
}

CreatureData::CreatureData(float sight, int hue)
{
	this->sight = sight;
	this->hue	= hue;

	// default design

	sd = {
		{{24, 24}, {}},
		{{8, 24}, {}},
		{{4, 24}, {}},
	};

	auto vec = {in::Connection{1, 6, 1}, in::Connection{1, 7, 1}};

	netStr = new in::NetworkStructure(2, totalSegs(sd) * 2 + 2, 0, totalSegs(sd), vec);

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

int CreatureData::totalSegs(std::vector<SegmentData> &segs)
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
