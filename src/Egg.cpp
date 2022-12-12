#include "../inc/Egg.hpp"

void Egg::setup(CreatureData &creatureData)
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
}

CreatureData &Egg::getCreatureData() // HACK lazy fix keeps deconstructing so just made reference
{
	return creatureData;
}

int Egg::getTimeLeft()
{
	return timeleft;
}
