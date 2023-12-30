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
            agl::Vec<float, 2> conv = force / (SIMSCALE * SIMSCALE *SIMSCALE);

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
		float				   rotation = 0;

		b2Body *phyBody;

		PhyRect() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			drawFunc = [&](agl::RenderWindow &window) {
				rect->setOffset(size * -.5);
				rect->setSize(size);
				rect->setPosition(position);
				rect->setColor(agl::Color::White);
				rect->setRotation({0, 0, agl::radianToDegree(rotation)});

				window.drawShape(*rect);
			};
		}

		void setup(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, b2World &world, b2BodyType type = b2_dynamicBody)
		{
			b2BodyDef bodyDef;
			bodyDef.type	 = type;
			bodyDef.position = PhysicsObj::scalePos(pos);
            bodyDef.fixedRotation = false;

			b2PolygonShape shapeDef;
			shapeDef.SetAsBox((size.x / 2) / SIMSCALE, (size.y / 2) / SIMSCALE);
		
            b2FixtureDef fixtureDef;
			fixtureDef.density	= 1;
			fixtureDef.friction = 1;
			fixtureDef.shape	= &shapeDef;

			// Now we have a body for our Box object
			phyBody = world.CreateBody(&bodyDef);
			// Lastly, assign the fixture
			phyBody->CreateFixture(&fixtureDef);

			this->size = size;
		}

        void sync()
        {
            position.x = phyBody->GetPosition().x * SIMSCALE;
            position.y = phyBody->GetPosition().y * SIMSCALE;
            rotation = -phyBody->GetAngle();
            std::cout << rotation << '\n';
        }
};
