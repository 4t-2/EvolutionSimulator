#pragma once

#include "Environment.hpp"
#include <AGL/agl.hpp>

class PhysicsObj : public BaseEntity
{
	public:
		agl::Vec<float, 2> posOffset;
		agl::Vec<float, 2> velocity;
		agl::Vec<float, 2> force;
		float			   radius = 1;
		float			   mass	  = 1;

		PhysicsObj(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}

		void updatePhysics()
		{
			velocity = velocity + (force * (1. / mass));
			force	 = {0, 0};

			position = position + posOffset;
			position = position + velocity;

			posOffset = {0, 0};
		}
};
