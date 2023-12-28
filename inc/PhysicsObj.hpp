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
		float			   mass = 1;
		float			   rotation;
		float			   angularVelocity;
		// F = I x α, where F is force (N), I is moment of inertia (kgm^2), and α is
		// angular acceleration (radians/s^2)
		PhysicsObj(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}

		void updatePhysics()
		{
			velocity = velocity + (force * (1. / mass));
			force	 = {0, 0};

			position = position + posOffset;
			position = position + velocity;

            rotation += angularVelocity;

			posOffset = {0, 0};
		}
};

class CanBeDrawn : public BaseEntity
{
	public:
		std::function<void(agl::RenderWindow &window)> drawFunc;

		CanBeDrawn(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}
};

class PhyCircle : public Entity<PhysicsObj, CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	position;
		bool				exists;
		static agl::Circle *circle;
		float				radius = 1;

		PhyCircle() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			mass   = 1;
			radius = 10;

			drawFunc = [&](agl::RenderWindow &window) {
				circle->setSize(agl::Vec<float, 2>{radius, radius});
				circle->setPosition(position);
				circle->setColor(agl::Color::White);

				window.drawShape(*circle);
			};
		}
};

class PhySquare : public Entity<PhysicsObj, CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	   position;
		bool				   exists;
		static agl::Rectangle *rect;
		float				   length = 20;

		float radToDeg()
		{
			return agl::radianToDegree(rotation);
		}

		PhySquare() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			mass = 1;

			drawFunc = [&](agl::RenderWindow &window) {
				rect->setOffset(agl::Vec<float, 2>{length, length} * -.5);
				rect->setSize(agl::Vec<float, 2>{length, length});
				rect->setPosition(position);
				rect->setColor(agl::Color::White);
				rect->setRotation({0, 0, radToDeg()});

				window.drawShape(*rect);
			};
		}
};
