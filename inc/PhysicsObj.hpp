#pragma once

#include "Environment.hpp"
#include <AGL/agl.hpp>
#include <chrono>

class PhysicsObj : public BaseEntity
{
	public:
		agl::Vec<float, 2> posOffset	   = {0, 0};
		agl::Vec<float, 2> velocity		   = {0, 0};
		agl::Vec<float, 2> force		   = {0, 0};
		float			   mass			   = 1;
		float			   rotation		   = 0;
		float			   angularVelocity = 0;
		float			   inertia		   = 1;

		// F = I x α, where F is force (N), I is moment of inertia (kgm^2), and α is
		// angular acceleration (radians/s^2)
		PhysicsObj(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}

		float radToDeg()
		{
			return agl::radianToDegree(rotation);
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
		agl::Vec<float, 2>	position = {0, 0};
		bool				exists	 = false;
		static agl::Circle *circle;
		float				radius = 1;

		PhyCircle() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			mass   = 1;
			radius = 10;

			inertia = 0.5 * mass * radius * radius;

			drawFunc = [&](agl::RenderWindow &window) {
				circle->setSize(agl::Vec<float, 2>{radius, radius});
				circle->setPosition(position);
				circle->setColor(agl::Color::White);
				circle->setRotation({0, 0, radToDeg()});

				window.drawShape(*circle);
			};
		}
};

class PhySquare : public Entity<PhysicsObj, CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	   position = {0, 0};
		bool				   exists	= false;
		static agl::Rectangle *rect;
		// float				   length = 20;
		float width	 = 80;
		float height = 20;

		PhySquare() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			mass = 1;

			inertia = (1 / 12.) * mass * (width * width + height * height);

			drawFunc = [&](agl::RenderWindow &window) {
				rect->setOffset(agl::Vec<float, 2>{width, height} * -.5);
				rect->setSize(agl::Vec<float, 2>{width, height});
				rect->setPosition(position);
				rect->setColor(agl::Color::White);
				rect->setRotation({0, 0, radToDeg()});

				window.drawShape(*rect);
			};
		}
};

class Collision
{
	public:
		// agl::Vec<float, 2> obj1Inter;
		// agl::Vec<float, 2> obj2Inter;
		// float			   overlap;
		// agl::Vec<float, 2> objOffset;

		static void checkCollision(PhySquare square, PhyCircle circle)
		{
		}

		static void resolveCollision(PhysicsObj &obj1, PhysicsObj &obj2, agl::Vec<float, 2> obj1Point,
									 agl::Vec<float, 2> obj2Point, agl::Vec<float, 2> normal, float restitution,
									 float overlap)
		{
			agl::Vec<float, 2> obj1PerpRadius = {-obj1Point.y, obj1Point.x};
			agl::Vec<float, 2> obj2PerpRadius = {-obj2Point.y, obj2Point.x};

			agl::Vec<float, 2> relVel = (obj2.velocity + (obj2PerpRadius * -obj2.angularVelocity)) -
										(obj1.velocity + (obj1PerpRadius * -obj1.angularVelocity));

			float impulse = (relVel * -(1 + restitution)).dot(normal);

			float add1 = std::pow(obj1PerpRadius.dot(normal), 2) / obj1.inertia;
			float add2 = std::pow(obj2PerpRadius.dot(normal), 2) / obj2.inertia;

			impulse /= (((1 / obj1.mass) + (1 / obj2.mass))) + add1 + add2;

			obj1.velocity -= normal * (impulse / obj1.mass);
			obj2.velocity += normal * (impulse / obj2.mass);

			obj1.angularVelocity += obj1PerpRadius.dot(normal * impulse) / obj1.inertia;
			obj2.angularVelocity -= obj2PerpRadius.dot(normal * impulse) / obj2.inertia;

			obj1.posOffset += normal * overlap;
			obj2.posOffset -= normal * overlap;
		}

		static agl::Vec<float, 2> closestPointToLine(agl::Vec<float, 2> a, agl::Vec<float, 2> b, agl::Vec<float, 2> p)
		{
			auto ab = b - a;
			auto ap = p - a;

			float proj	  = ap.dot(ab);
			float abLenSq = ab.dot(ab);
			float d		  = proj / abLenSq;

			if (d <= 0)
			{
                std::cout << "a" << '\n';
				return a;
			}
			else if (d >= 1)
			{
                std::cout << "b" << '\n';
				return b;
			}
			else
			{
                std::cout << "c" << '\n';
				return a + ab * d;
			}
		}
};
