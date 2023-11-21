#pragma once

#include <AGL/agl.hpp>

class PhysicsObj
{
	public:
		agl::Vec<float, 2> position;
		agl::Vec<float, 2> posOffset;
		agl::Vec<float, 2> velocity;
		agl::Vec<float, 2> force;
		float			   radius = 1;
		float			   mass	  = 1;
};
