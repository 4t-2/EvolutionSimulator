#include "../inc/Egg.hpp"

void Egg::setup(CreatureData *creatureData)
{
	this->creatureData = creatureData;

	timeleft = 100;

	return;
}

void Egg::update()
{
	timeleft--;
}

void Egg::clear()
{
	timeleft = 0;
	delete creatureData;
}

CreatureData *Egg::getCreatureData()
{
	return creatureData;
}

int Egg::getTimeLeft()
{
	return timeleft;
}
