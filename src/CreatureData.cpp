#include "../inc/CreatureData.hpp"

CreatureData::CreatureData()
{
	connection = nullptr;

	return;
}

CreatureData::CreatureData(float sight, float speed, float size, int hue, int totalConnections)
{
	this->sight			   = sight;
	this->speed			   = speed;
	this->size			   = size;
	this->totalConnections = totalConnections;
	this->hue			   = hue;
	this->startEnergy	   = 25 * size * size * size;

	connection = new in::Connection[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		connection[i].exists = false;
	}

	return;
}

CreatureData::CreatureData(const CreatureData &creatureData)
{
	totalConnections = creatureData.totalConnections;

	delete[] connection;
	connection = new in::Connection[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		connection[i] = creatureData.connection[i];
	}

	sight		= creatureData.sight;
	speed		= creatureData.speed;
	size		= creatureData.size;
	hue			= creatureData.hue;
	startEnergy = creatureData.startEnergy;
	preference	= creatureData.preference;
	metabolism	= creatureData.metabolism;
	useNEAT		= creatureData.useNEAT;
	usePG		= creatureData.usePG;
}

void CreatureData::operator=(CreatureData &creatureData)
{
	totalConnections = creatureData.totalConnections;

	delete[] connection;
	connection = new in::Connection[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		connection[i] = creatureData.connection[i];
	}

	sight		= creatureData.sight;
	speed		= creatureData.speed;
	size		= creatureData.size;
	hue			= creatureData.hue;
	startEnergy = creatureData.startEnergy;
	preference	= creatureData.preference;
	metabolism	= creatureData.metabolism;
	useNEAT		= creatureData.useNEAT;
	usePG		= creatureData.usePG;
}

CreatureData::~CreatureData()
{
	delete[] connection;

	return;
}

void CreatureData::setConnection(int index, int startNode, int endNode, float weight)
{
	connection[index].startNode = startNode;
	connection[index].endNode	= endNode;
	connection[index].weight	= weight;
	connection[index].exists	= true;

	return;
}

void CreatureData::setNetwork(in::NetworkStructure &structure)
{
	for (int i = 0; i < totalConnections; i++)
	{
		connection[i] = structure.connection[i];
	}
}
