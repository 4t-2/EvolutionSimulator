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

void Egg::setPosition(agl::Vec<float, 2> position)
{
	this->position = position;
}

agl::Vec<float, 2> Egg::getPosition()
{
	return position;
}

CreatureData &Egg::getCreatureData() // HACK lazy fix keeps deconstructing so just made reference
{
	return creatureData;
}

int Egg::getTimeLeft()
{
	return timeleft;
}
