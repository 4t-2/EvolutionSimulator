#include "../inc/CreatureData.hpp"

CreatureData::CreatureData()
{
	connection = nullptr;

	return;
}

CreatureData::CreatureData(float sight, float speed, float size, int totalConnections)
{
	this->sight			   = sight;
	this->speed			   = speed;
	this->size			   = size;
	this->totalConnections = totalConnections;

	connection = new Connection[totalConnections];

	return;
}

CreatureData::CreatureData(const CreatureData &creatureData)
{
	totalConnections = creatureData.totalConnections;
	delete[] connection;
	connection = new Connection[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		connection[i] = creatureData.connection[i];
	}

	sight = creatureData.sight;
	speed = creatureData.speed;
	size = creatureData.size;
}

void CreatureData::operator=(CreatureData &creatureData)
{
	totalConnections = creatureData.totalConnections;
	delete[] connection;
	connection = new Connection[totalConnections];

	for (int i = 0; i < totalConnections; i++)
	{
		connection[i] = creatureData.connection[i];
	}

	sight = creatureData.sight;
	speed = creatureData.speed;
	size = creatureData.size;
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

	return;
}

int CreatureData::getTotalConnections()
{
	return totalConnections;
}

Connection *CreatureData::getConnection()
{
	return connection;
}

float CreatureData::getSight()
{
	return sight;
}
float CreatureData::getSpeed()
{
	return speed;
}
float CreatureData::getSize()
{
	return size;
}
