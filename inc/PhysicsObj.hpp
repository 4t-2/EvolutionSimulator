#pragma once

#include "Environment.hpp"
#include <AGL/agl.hpp>
#include <chrono>

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

class PhyCircle : public Entity<PhysicsObj>
{
	public:
		agl::Vec<float, 2>							   position;
		bool										   exists;
		static agl::Circle							  *circle;
		std::function<void(agl::RenderWindow &window)> drawFunc;

		PhyCircle() : Entity<PhysicsObj>(exists, position)
		{
			mass = 1;
			radius = 10;

			drawFunc = [&](agl::RenderWindow &window) {
				circle->setSize(agl::Vec<float, 2>{radius, radius});
				circle->setPosition(position);
				circle->setColor(agl::Color::White);

				window.drawShape(*circle);
			};
		}
};
