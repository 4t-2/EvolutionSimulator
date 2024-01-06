#pragma once

#include "Environment.hpp"
#include <AGL/agl.hpp>
#include <box2d/box2d.h>

#define SIMSCALE 10.f

class PhysicsObj : public BaseEntity
{
	public:
		PhysicsObj(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}

		static b2Vec2 scalePos(agl::Vec<float, 2> pos)
		{
			agl::Vec<float, 2> conv = pos / SIMSCALE;

			return {conv.x, conv.y};
		}

		static b2Vec2 scaleVel(agl::Vec<float, 2> vel)
		{
			agl::Vec<float, 2> conv = vel / SIMSCALE;

			return {conv.x, conv.y};
		}

		static float scaleMass(float mass)
		{
			float conv = mass / (SIMSCALE * SIMSCALE);

			return conv;
		}

		static b2Vec2 scaleGrav(agl::Vec<float, 2> grav)
		{
			agl::Vec<float, 2> conv = grav / (SIMSCALE);

			return {conv.x, conv.y};
		}

		static b2Vec2 scaleForce(agl::Vec<float, 2> force)
		{
			agl::Vec<float, 2> conv = force / (SIMSCALE * SIMSCALE * SIMSCALE);

			return {conv.x, conv.y};
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

class PhyCircle : public Entity<CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	position = {0, 0};
		bool				exists	 = false;
		static agl::Circle *circle;
		float				radius = 1;

		PhyCircle() : Entity<CanBeDrawn>(exists, position)
		{
			drawFunc = [&](agl::RenderWindow &window) {
				circle->setSize(agl::Vec<float, 2>{radius, radius});
				circle->setPosition(position);
				circle->setColor(agl::Color::White);
				circle->setRotation({0, 0, 0});

				window.drawShape(*circle);
			};
		}

		void setBox()
		{
		}
};

class PhyOld : public BaseEntity
{
	public:
		agl::Vec<float, 2> posOffset	   = {0, 0};
		agl::Vec<float, 2> velocity		   = {0, 0};
		agl::Vec<float, 2> force		   = {0, 0};
		float			   mass			   = 1;
		float			   rotation		   = 0;
		float			   angularVelocity = 0;
		float			   inertia		   = 1;
		float			   invMass		   = 1;
		float			   invInertia	   = 1;
		float			   angularForce	   = 0;

		PhyOld(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}
};

class PhyRect : public Entity<PhysicsObj, CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	   position = {0, 0};
		bool				   exists	= false;
		static agl::Rectangle *rect;
		agl::Vec<float, 2>	   size;
		float				   rotation	 = 0;
		agl::Color			   color	 = agl::Color::White;
		agl::Color			   realColor = agl::Color::White;

		b2Body *phyBody;

		PhyRect() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			drawFunc = [&](agl::RenderWindow &window) {
				rect->setOffset(size * -.5);
				rect->setSize(size);
				rect->setPosition(position);
				rect->setColor(color);
				rect->setRotation({0, 0, agl::radianToDegree(rotation)});

				window.drawShape(*rect);
			};
		}

		void setup(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, float rotation, b2World &world,
				   b2BodyType type = b2_dynamicBody, int groupIndex = 0)
		{
			b2BodyDef bodyDef;
			bodyDef.type		  = type;
			bodyDef.position	  = PhysicsObj::scalePos(pos);
			bodyDef.fixedRotation = false;
			bodyDef.angle		  = rotation;

			b2PolygonShape shapeDef;
			shapeDef.SetAsBox((size.x / 2) / SIMSCALE, (size.y / 2) / SIMSCALE);

			b2FixtureDef fixtureDef;
			fixtureDef.density			 = 1;
			fixtureDef.friction			 = 1;
			fixtureDef.shape			 = &shapeDef;
			fixtureDef.filter.groupIndex = groupIndex;

			// Now we have a body for our Box object
			phyBody = world.CreateBody(&bodyDef);
			// Lastly, assign the fixture
			phyBody->CreateFixture(&fixtureDef);

			this->size = size;

            // phyBody->ApplyTorque(1, true);

			sync();
		}

		void sync()
		{
			position.x = phyBody->GetPosition().x * SIMSCALE;
			position.y = phyBody->GetPosition().y * SIMSCALE;
			rotation   = -phyBody->GetAngle();
		}
};

struct JointDef
{
		b2RevoluteJoint def;
};

class PhyJoint
{
	public:
		b2RevoluteJoint *joint;
		PhyRect			*start;
		PhyRect			*end;

		agl::Vec<float, 2> local1;
		agl::Vec<float, 2> local2;

		PhyJoint()
		{
		}

		void setup(PhyRect &rect1, PhyRect &rect2, agl::Vec<float, 2> local1, agl::Vec<float, 2> local2, b2World &world,
				   float speed = PI / 50)
		{
			start		 = &rect1;
			end			 = &rect2;
			this->local1 = local1;
			this->local2 = local2;

			b2RevoluteJointDef revoluteJointDef;
			revoluteJointDef.bodyA			  = rect1.phyBody;
			revoluteJointDef.bodyB			  = rect2.phyBody;
			revoluteJointDef.collideConnected = false;

			b2Vec2 b2l1 = PhysicsObj::scalePos(local1);
			b2Vec2 b2l2 = PhysicsObj::scalePos(local2);

			revoluteJointDef.localAnchorA.Set(b2l1.x, b2l1.y);
			revoluteJointDef.localAnchorB.Set(b2l2.x, b2l2.y);

			revoluteJointDef.enableLimit = true;
			revoluteJointDef.upperAngle	 = PI / 2;
			revoluteJointDef.lowerAngle	 = -PI / 2;

			revoluteJointDef.enableMotor	= true;
			revoluteJointDef.maxMotorTorque = 100;
			// revoluteJointDef.motorSpeed		= 1. / 6;

			revoluteJointDef.referenceAngle = rect1.rotation - rect2.rotation;

			joint = (b2RevoluteJoint *)world.CreateJoint(&revoluteJointDef);
		}
};
