#pragma once

#include "../lib/AGL/agl.hpp"

class Food
{
	public:
		int id;
		agl::Vec<float, 2> position;
		float energy;
};
