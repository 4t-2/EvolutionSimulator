#pragma once

#include "Environment.hpp"
#include <AGL/agl.hpp>
#include <box2d/box2d.h>
#include <chrono>
#include <list>

#define SIMSCALE 10.f

class NewBox
{
	public:
		bool			   forcable			   = false;
		bool			   dynamic			   = false;
		agl::Vec<float, 2> position			   = {0, 0};
		agl::Vec<float, 2> size				   = {0, 0};
		agl::Vec<float, 2> posOffset		   = {0, 0};
		agl::Vec<float, 2> velocity			   = {0, 0};
		agl::Vec<float, 2> acceleration		   = {0, 0};
		float			   mass				   = 1;
		float			   rotation			   = 0;
		float			   angularVelocity	   = 0;
		float			   inertia			   = 1;
		float			   invMass			   = 1;
		float			   invInertia		   = 1;
		float			   angularAcceleration = 0;
		int				   totalContacts	   = 0;
        float rotOffset = 0;

		NewBox			  *rootConnect = nullptr;
		agl::Vec<float, 2> local1;
		agl::Vec<float, 2> local2;

		agl::Vec<float, 2> GetPosition()
		{
			return position;
		}

		float GetAngle()
		{
			return rotation;
		}

		agl::Vec<float, 2> GetLinearVelocity()
		{
			return velocity;
		}

		void calcInertia(float value)
		{
			inertia = (1 / 12.) * mass * (size.x * size.x + size.y * size.y);
		}

		void setMass(float value)
		{
			if (value != 0)
			{
				mass = value;
				calcInertia(value);

				invMass = 1 / mass;

				if (inertia == 0)
				{
					invInertia = 0;
				}
				else
				{
					invInertia = 1 / inertia;
				}
			}
			else
			{
				mass	   = 0;
				inertia	   = 0;
				invMass	   = 0;
				invInertia = 0;
			}
		}

		float radToDeg()
		{
			return agl::radianToDegree(rotation);
		}
		void updatePhysics()
		{
			velocity			= velocity + acceleration;
			angularVelocity		= angularVelocity + angularAcceleration;
			acceleration		= {0, 0};
			angularAcceleration = 0;

			position = position + velocity;
			position += posOffset;
			posOffset = {0, 0};

			rotation += angularVelocity;
            rotation += rotOffset;
            rotOffset = 0;
		}

		void SetTransform(agl::Vec<float, 2> pos, float rot)
		{
			position = pos;
			rotation = rot;
		}

		void ApplyForceToCenter(b2Vec2 force, bool)
		{
			agl::Vec<float, 2> f;
			f.x = force.x;
			f.y = force.y;

			acceleration += f * invMass;
		}

		bool pointInside(b2Vec2 point)
		{
			if (point.x < (position.x + size.x / 2) && point.x > (position.x - size.x / 2) &&
				point.y < (position.y + size.y / 2) && point.y > (position.y - size.y / 2))
			{
				return true;
			}

			return false;
		}
};

class World
{
	public:
		std::list<NewBox>  stuff;
		agl::Vec<float, 2> grav;

		agl::Vec<float, 2> mpos;

		void addJoint(NewBox &b1, agl::Vec<float, 2> local1, NewBox &b2, agl::Vec<float, 2> local2)
		{
			b1.rootConnect = &b2;
			b1.local1	   = local1;
			b1.local2	   = local2;
		}

		struct Collision
		{
				NewBox			  *b1;
				NewBox			  *b2;
				agl::Vec<float, 2> inter1  = {0, 0};
				agl::Vec<float, 2> inter2  = {0, 0};
				agl::Vec<float, 2> normal  = {0, 0};
				bool			   collide = false;
				float			   depth   = 0;
				bool			   offset  = true;
		};

		std::list<Collision> collisionList;

		World(b2Vec2 grav)
		{
			this->grav.x = grav.x;
			this->grav.y = grav.y;
		}

		void DestroyBody(NewBox *box)
		{
			for (auto it = stuff.begin(); it != stuff.end(); it++)
			{
				if (box == &*it)
				{
					stuff.erase(it);
					return;
				}
			}
		}

		void SetGravity(b2Vec2 grav)
		{
			this->grav.x = grav.x;
			this->grav.y = grav.y;
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
				return a;
			}
			else if (d >= 1)
			{
				return b;
			}
			else
			{
				return a + ab * d;
			}
		}

		NewBox *CreateBody(b2BodyDef *def, agl::Vec<float, 2> size)
		{
			stuff.emplace_back();

			stuff.back().position.x = def->position.x;
			stuff.back().position.y = def->position.y;
			stuff.back().size.x		= size.x;
			stuff.back().size.y		= size.y;

			if (def->type == b2_dynamicBody)
			{
				stuff.back().dynamic = true;
				stuff.back().setMass(1);
			}
			else
			{
				stuff.back().dynamic = false;
				stuff.back().setMass(0);
			}

			return &stuff.back();
		}

		agl::Vec<float, 2> AABBnormal(NewBox &box1, NewBox &box2)
		{
			// Calculate the half-sizes of the boxes
			agl::Vec<float, 2> halfSize1 = box1.size * 0.5;
			agl::Vec<float, 2> halfSize2 = box2.size * 0.5;

			// Calculate the centers of the boxes
			agl::Vec<float, 2> center1 = box1.position;
			agl::Vec<float, 2> center2 = box2.position;

			// Calculate the difference in centers
			agl::Vec<float, 2> difference	 = center2 - center1;
			agl::Vec<float, 2> absDifference = {abs(difference.x), abs(difference.y)};

			// Calculate the overlap on each axis
			agl::Vec<float, 2> overlap = absDifference - (halfSize1 + halfSize2);

			// Calculate the normal
			agl::Vec<float, 2> normal;
			if (overlap.x > 0 || overlap.y > 0)
			{
				normal = {0, 0};
			}
			else if (overlap.x > overlap.y)
			{
				if (difference.x < 0)
				{
					normal = {-1, 0};
				}
				else
				{
					normal = {1, 0};
				}
			}
			else
			{
				if (difference.y < 0)
				{
					normal = {0, -1};
				}
				else
				{
					normal = {0, 1};
				}
			}

			return normal;
		}

		struct PolyShape
		{
				std::vector<agl::Vec<float, 2>> points;
				std::vector<agl::Vec<float, 2>> normals;
		};

		void boxToPoly(NewBox &b1, PolyShape &shape)
		{
			agl::Mat4f rot;
			rot.rotateZ(-b1.radToDeg());

			shape.points.resize(4);
			shape.points[0] = b1.position + rot * agl::Vec<float, 2>{-b1.size.x, -b1.size.y} / 2;
			shape.points[1] = b1.position + rot * agl::Vec<float, 2>{b1.size.x, -b1.size.y} / 2;
			shape.points[2] = b1.position + rot * agl::Vec<float, 2>{b1.size.x, b1.size.y} / 2;
			shape.points[3] = b1.position + rot * agl::Vec<float, 2>{-b1.size.x, b1.size.y} / 2;

			shape.normals.resize(4);
			shape.normals[0] = rot * agl::Vec<float, 2>{0, -1};
			shape.normals[1] = rot * agl::Vec<float, 2>{1, 0};
			shape.normals[2] = rot * agl::Vec<float, 2>{0, 1};
			shape.normals[3] = rot * agl::Vec<float, 2>{-1, 0};
		}

		struct BareData
		{
				agl::Vec<float, 2> normal;
				agl::Vec<float, 2> online;
				float			   depth = 0;
                agl::Vec<float, 2> inter1;
                agl::Vec<float, 2> inter2;
		};

		float cross2D(agl::Vec<float, 2> a, agl::Vec<float, 2> b)
		{
			return a.x * b.y - a.y * b.x;
		}

		void applyForce(NewBox &b, agl::Vec<float, 2> force, agl::Vec<float, 2> point)
		{
			b.acceleration += force * b.invMass;
			b.angularAcceleration += cross2D(point - b.position, force) * b.invInertia;
		}

		void processJoint(NewBox &b1, BareData &bare)
		{
			agl::Mat4f rot;
			rot.rotateZ(-b1.radToDeg());

			agl::Vec<float, 2> local1 = b1.position + (rot * b1.local1);
			rot.rotateZ(-b1.rootConnect->radToDeg());
			agl::Vec<float, 2> local2 = b1.rootConnect->position + (rot * b1.local2);

			agl::Vec<float, 2> offset = local1 - local2;
			agl::Vec<float, 2> normal = offset.normalized();
			float			   dist	  = offset.length();

			bare.normal = normal;
			bare.depth	= dist;
            bare.inter1 = local1;
            bare.inter2 = local2;
		}

		template <bool useBare = true>
		bool pointInBox(PolyShape &shape, agl::Vec<float, 2> point, BareData &bare = *(BareData *)nullptr)
		{
			float			   depth = MAXFLOAT;
			int				   index = 0;
			agl::Vec<float, 2> online;

			for (int i = 0; i < shape.points.size(); i++)
			{
				agl::Vec<float, 2> c =
					closestPointToLine(shape.points[i], shape.points[(i + 1) % shape.points.size()], point);

				float dot = -shape.normals[i].dot(c - point);

				if (dot > 0)
				{
					return false;
				}

				float d = (c - point).length();

				if constexpr (useBare)
				{
					if (d < depth)
					{
						index  = i;
						online = c;
						depth  = d;
					}
				}
				if (i == shape.points.size() - 1)
				{
					if constexpr (useBare)
					{
						bare.online = online;
						bare.normal = shape.normals[index];
						bare.depth	= depth;
					}
					return true;
				}
			}

			return false;
		}

		agl::Vec<float, 2> perp(agl::Vec<float, 2> vec)
		{
			return {-vec.y, vec.x};
		}

		void testRes(Collision &collision, int divider)
		{
			agl::Vec<float, 2> rp1	  = perp(collision.b1->position - collision.inter1);
			agl::Vec<float, 2> rp2	  = perp(collision.b2->position - collision.inter2);
			agl::Vec<float, 2> offset = (collision.inter1 - collision.inter2);

			float restitution = 0;
			auto  top		  = (offset * -(1 + restitution)).dot(collision.normal);

			float botl1 = std::pow(rp1.dot(collision.normal), 2) * collision.b1->invInertia;
			float botl2 = std::pow(rp2.dot(collision.normal), 2) * collision.b2->invInertia;

			auto bottom = collision.normal.dot(collision.normal * (collision.b1->invMass + collision.b2->invMass)) +
						  botl1 + botl2;
			float impulse = top / bottom;

			impulse /= divider;

			agl::Vec<float, 2> acc1 = collision.normal * (impulse * collision.b1->invMass);
			agl::Vec<float, 2> acc2 = collision.normal * (impulse * -collision.b2->invMass);
			collision.b1->posOffset += acc1;
			collision.b2->posOffset += acc2;
            std::cout << offset << '\n';
            std::cout << acc1 << '\n';
            std::cout << acc2 << '\n';
			collision.b1->rotOffset -= rp1.dot(collision.normal * impulse) * collision.b1->invInertia;
			collision.b2->rotOffset += rp2.dot(collision.normal * impulse) * collision.b2->invInertia;
		}

		void resolve(Collision &collision, int divider)
		{
			agl::Vec<float, 2> rp1	  = perp(collision.b1->position - collision.inter1);
			agl::Vec<float, 2> rp2	  = perp(collision.b2->position - collision.inter2);
			agl::Vec<float, 2> relVel = (collision.b1->velocity + (rp1 * -collision.b1->angularVelocity)) -
										(collision.b2->velocity + (rp2 * -collision.b2->angularVelocity));

			float restitution = 0;
			auto  top		  = (relVel * -(1 + restitution)).dot(collision.normal);

			float botl1 = std::pow(rp1.dot(collision.normal), 2) * collision.b1->invInertia;
			float botl2 = std::pow(rp2.dot(collision.normal), 2) * collision.b2->invInertia;

			auto bottom = collision.normal.dot(collision.normal * (collision.b1->invMass + collision.b2->invMass)) +
						  botl1 + botl2;
			float impulse = top / bottom;

			impulse /= divider;

			agl::Vec<float, 2> acc1 = collision.normal * (impulse * collision.b1->invMass);
			agl::Vec<float, 2> acc2 = collision.normal * (impulse * -collision.b2->invMass);
			collision.b1->acceleration += acc1;
			collision.b2->acceleration += acc2;

			collision.b1->angularAcceleration -= rp1.dot(collision.normal * impulse) * collision.b1->invInertia;
			collision.b2->angularAcceleration += rp2.dot(collision.normal * impulse) * collision.b2->invInertia;

            testRes(collision, divider);
			// if (collision.offset)
			// {
			//
			// 	if (collision.b1->dynamic && collision.b2->dynamic)
			// 	{
			// 		float total = collision.b1->mass + collision.b2->mass;
			// 		collision.b1->posOffset -= collision.normal * collision.depth * (collision.b2->mass / total);
			// 		collision.b2->posOffset += collision.normal * collision.depth * (collision.b1->mass / total);
			// 	}
			// 	else if (collision.b1->dynamic)
			// 	{
			// 		collision.b1->posOffset -= collision.normal * collision.depth;
			// 	}
			// 	else
			// 	{
			// 		collision.b2->posOffset += collision.normal * collision.depth;
			// 	}
			// }
		}

		void collide(NewBox &b1, NewBox &b2)
		{
			PolyShape s1;
			PolyShape s2;
			// CHECK COLLISION
			boxToPoly(b1, s1);
			boxToPoly(b2, s2);

			std::vector<Collision> collision;

			auto compCol = [&](NewBox &a, NewBox &b, PolyShape &sa, PolyShape &sb) {
				for (agl::Vec<float, 2> &point : sb.points)
				{
					BareData bare;
					if (pointInBox(sa, point, bare))
					{
						collision.emplace_back();
						collision.back().b1		= &a;
						collision.back().b2		= &b;
						collision.back().normal = bare.normal;
						collision.back().inter1 = bare.online;
						collision.back().inter2 = point;
						collision.back().depth	= bare.depth;
					}
				}
			};

			if (b1.rootConnect == &b2)
			{
				BareData bare;
				processJoint(b1, bare);

				if (bare.depth != 0)
				{
					collision.emplace_back();
					collision.back().b1		= &b1;
					collision.back().b2		= &b2;
					collision.back().normal = bare.normal;
					collision.back().inter1 = bare.inter1;
					collision.back().inter2 = bare.inter2;
					collision.back().depth	= bare.depth;
				}
			}
			else if (b2.rootConnect == &b1)
			{
			}
			else
			{
				compCol(b1, b2, s1, s2);
				compCol(b2, b1, s2, s1);
			}

			for (Collision &c : collision)
			{
				std::cout << "\nRESOLVE\n";
				std::cout << "depth " << c.depth << '\n';
				std::cout << "norm " << c.normal << '\n';
				resolve(c, collision.size());
			}
		}

		void Step()
		{
			for (auto x = stuff.begin(); x != stuff.end(); x++)
			{
				for (auto y = std::next(x, 1); y != stuff.end(); y++)
				{
					collide(*x, *y);
				}
			}

			for (auto x = stuff.begin(); x != stuff.end(); x++)
			{
				x->acceleration += grav * x->invMass;
				x->updatePhysics();
			}
		}
};

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

		NewBox *phyBody;

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

		void setup(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, float rotation, World &world,
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
			phyBody			  = world.CreateBody(&bodyDef, size / SIMSCALE);
			phyBody->rotation = rotation;

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

		agl::Vec<float, 2> getVel()
		{
			return {phyBody->GetLinearVelocity().x * SIMSCALE, phyBody->GetLinearVelocity().y * SIMSCALE};
		}
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

		void setup(PhyRect &rect1, PhyRect &rect2, agl::Vec<float, 2> local1, agl::Vec<float, 2> local2, World &world,
				   float speed = PI / 50)
		{
			world.addJoint(*(rect1.phyBody), local1 / SIMSCALE, *(rect2.phyBody), local2 / SIMSCALE);

			return;
			start		 = &rect1;
			end			 = &rect2;
			this->local1 = local1;
			this->local2 = local2;

			b2RevoluteJointDef revoluteJointDef;
			// revoluteJointDef.bodyA			  = rect1.phyBody;
			// revoluteJointDef.bodyB			  = rect2.phyBody;
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

			// joint = (b2RevoluteJoint *)world.CreateJoint(&revoluteJointDef);
		}
};

// OLD
// -----------------------------------------------------------------------------------------

#if 0
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
		float			   invMass		   = 1;
		float			   invInertia	   = 1;
		float			   angularForce	   = 0;

		virtual void calcInertia(float value){};

		void setMass(float value)
		{
			if (value != 0)
			{
				mass = value;
				calcInertia(value);

				invMass = 1 / mass;

				if (inertia == 0)
				{
					invInertia = 0;
				}
				else
				{
					invInertia = 1 / inertia;
				}
			}
			else
			{
				mass	   = 0;
				inertia	   = 0;
				invMass	   = 0;
				invInertia = 0;
			}
		}

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
			velocity		= velocity + (force * invMass);
			angularVelocity = angularVelocity + (angularForce * invInertia);
			force			= {0, 0};
			angularForce	= 0;

			position = position + velocity;

			rotation += angularVelocity;
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
			radius = 10;

			setMass(1);

			drawFunc = [&](agl::RenderWindow &window) {
				circle->setSize(agl::Vec<float, 2>{radius, radius});
				circle->setPosition(position);
				circle->setColor(agl::Color::White);
				circle->setRotation({0, 0, radToDeg()});

				window.drawShape(*circle);
			};
		}

		void calcInertia(float value) override
		{
			// inertia = 0.5 * mass * radius * radius;
			inertia = 0;
		}
};

class PhyRect : public Entity<PhysicsObj, CanBeDrawn>
{
	public:
		agl::Vec<float, 2>	   position = {0, 0};
		bool				   exists	= false;
		static agl::Rectangle *rect;
		// float				   length = 20;
		float width	 = 80;
		float height = 20;

		PhyRect() : Entity<PhysicsObj, CanBeDrawn>(exists, position)
		{
			setMass(1);

			drawFunc = [&](agl::RenderWindow &window) {
				rect->setOffset(agl::Vec<float, 2>{width, height} * -.5);
				rect->setSize(agl::Vec<float, 2>{width, height});
				rect->setPosition(position);
				rect->setColor(agl::Color::White);
				rect->setRotation({0, 0, radToDeg()});

				window.drawShape(*rect);
			};
		}

		void calcInertia(float value) override
		{
			inertia = (1 / 12.) * mass * (width * width + height * height);
		}
};

class SimpleCircle
{
	public:
		agl::Vec<float, 2> position;
		float			   radius;
};

class Collision
{
	public:
		agl::Vec<float, 2> obj1Inter = {0, 0};
		agl::Vec<float, 2> obj2Inter = {0, 0};
		agl::Vec<float, 2> normal	 = {0, 0};
		float			   overlap	 = 0;
		bool			   contact	 = false;

		void checkCollision(SimpleCircle circle, SimpleCircle otherCircle)
		{
			agl::Vec<float, 2> offset = circle.position - otherCircle.position;

			float dist = offset.length();

			overlap = (otherCircle.radius + circle.radius) - dist;

			if (overlap > 0)
			{
				normal	  = offset.normalized();
				obj1Inter = normal * circle.radius;
				obj2Inter = normal * -otherCircle.radius;

				contact = true;
			}
		}

		void checkCollision(PhyCircle &circle, PhyCircle &otherCircle)
		{
			checkCollision({circle.position, circle.radius}, {otherCircle.position, otherCircle.radius});
		}

		void checkCollision(PhyRect &rect, PhyCircle &circle)
		{
			agl::Vec<float, 2> vert[4];

			vert[0] = agl::Vec<float, 2>{rect.width / 2, rect.height / 2};
			vert[1] = agl::Vec<float, 2>{-rect.width / 2, rect.height / 2};
			vert[2] = agl::Vec<float, 2>{-rect.width / 2, -rect.height / 2};
			vert[3] = agl::Vec<float, 2>{rect.width / 2, -rect.height / 2};

			agl::Mat4f rot;
			rot.rotateZ(rect.radToDeg());

			vert[0] = rot * vert[0];
			vert[1] = rot * vert[1];
			vert[2] = rot * vert[2];
			vert[3] = rot * vert[3];

			vert[0] += rect.position;
			vert[1] += rect.position;
			vert[2] += rect.position;
			vert[3] += rect.position;

			agl::Vec<float, 2> closest;
			float			   closeDist = MAXFLOAT;

			for (int i = 0; i < 3; i++)
			{
				agl::Vec<float, 2> point = Collision::closestPointToLine(vert[i], vert[i + 1], circle.position);

				float dist = (point - circle.position).length();

				if (dist < closeDist)
				{
					closeDist = dist;
					closest	  = point;
				}
			}

			{
				agl::Vec<float, 2> point = Collision::closestPointToLine(vert[3], vert[0], circle.position);
				float			   dist	 = (point - circle.position).length();

				if (dist < closeDist)
				{
					closeDist = dist;
					closest	  = point;
				}
			}

			overlap = circle.radius - closeDist;

			if (overlap > 0)
			{
				obj1Inter = closest - rect.position;
				obj2Inter = closest - rect.position;
				normal	  = (closest - circle.position).normalized();
				contact	  = true;

				if (std::isnan(normal.x))
				{
					normal = {1, 0};
				}
			}
		}

		void checkCollision(PhyRect &rect, PhyRect &otherRect)
		{
			agl::Vec<float, 2> vertRect[4];

			vertRect[0] = agl::Vec<float, 2>{rect.width / 2, rect.height / 2};
			vertRect[1] = agl::Vec<float, 2>{-rect.width / 2, rect.height / 2};
			vertRect[2] = agl::Vec<float, 2>{-rect.width / 2, -rect.height / 2};
			vertRect[3] = agl::Vec<float, 2>{rect.width / 2, -rect.height / 2};

			{
				agl::Mat4f rot;
				rot.rotateZ(rect.radToDeg());

				vertRect[0] = rot * vertRect[0];
				vertRect[1] = rot * vertRect[1];
				vertRect[2] = rot * vertRect[2];
				vertRect[3] = rot * vertRect[3];
			}

			vertRect[0] += rect.position;
			vertRect[1] += rect.position;
			vertRect[2] += rect.position;
			vertRect[3] += rect.position;

			agl::Vec<float, 2> vertOtherRect[4];

			vertOtherRect[0] = agl::Vec<float, 2>{otherRect.width / 2, otherRect.height / 2};
			vertOtherRect[1] = agl::Vec<float, 2>{-otherRect.width / 2, otherRect.height / 2};
			vertOtherRect[2] = agl::Vec<float, 2>{-otherRect.width / 2, -otherRect.height / 2};
			vertOtherRect[3] = agl::Vec<float, 2>{otherRect.width / 2, -otherRect.height / 2};

			{
				agl::Mat4f rot;
				rot.rotateZ(otherRect.radToDeg());

				vertOtherRect[0] = rot * vertOtherRect[0];
				vertOtherRect[1] = rot * vertOtherRect[1];
				vertOtherRect[2] = rot * vertOtherRect[2];
				vertOtherRect[3] = rot * vertOtherRect[3];
			}

			vertOtherRect[0] += otherRect.position;
			vertOtherRect[1] += otherRect.position;
			vertOtherRect[2] += otherRect.position;
			vertOtherRect[3] += otherRect.position;

			struct PointWithDist
			{
					agl::Vec<float, 2> point;
					float			   dist;
			};

			auto findClosest = [](agl::Vec<float, 2> vert[4], agl::Vec<float, 2> pointVec) -> PointWithDist {
				agl::Vec<float, 2> closest;
				float			   closeDist = MAXFLOAT;

				for (int i = 0; i < 3; i++)
				{
					agl::Vec<float, 2> point = Collision::closestPointToLine(vert[i], vert[i + 1], pointVec);

					float dist = (point - pointVec).length();

					if (dist < closeDist)
					{
						closeDist = dist;
						closest	  = point;
					}
				}

				{
					agl::Vec<float, 2> point = Collision::closestPointToLine(vert[3], vert[0], pointVec);

					float dist = (point - pointVec).length();

					if (dist < closeDist)
					{
						closeDist = dist;
						closest	  = point;
					}
				}

				return {closest, closeDist};
			};

			PointWithDist pwdRect = {{0, 0}, MAXFLOAT};

			for (int i = 0; i < 4; i++)
			{
				PointWithDist pwdNew = findClosest(vertRect, vertOtherRect[i]);

				if (pwdNew.dist < pwdRect.dist)
				{
					pwdRect = pwdNew;
                    std::cout << "first "<< i << '\n';
				}
			}

			PointWithDist pwdOtherRect = {{0, 0}, MAXFLOAT};

			for (int i = 0; i < 4; i++)
			{
				PointWithDist pwdNew = findClosest(vertOtherRect, vertRect[i]);

				if (pwdNew.dist < pwdOtherRect.dist)
				{
					pwdOtherRect = pwdNew;
                    std::cout << "second "<< i << '\n';
				}
			}

			normal = (pwdRect.point - pwdOtherRect.point).normalized();

			SimpleCircle circle;
			circle.position = pwdRect.point + (normal * -10);
			circle.radius	= 10;

			SimpleCircle otherCircle;
			circle.position = pwdOtherRect.point + (normal * 10);
			circle.radius	= 10;

			// Collision c;

			// c.checkCollision(circle, otherCircle);

            checkCollision(circle, otherCircle);

            std::cout << '\n';
            std::cout << pwdOtherRect.point << '\n';
            std::cout << pwdRect.point << '\n';
            std::cout << (pwdOtherRect.point - pwdRect.point).length() << '\n';
		}

		void resolveCollision(PhysicsObj &obj1, PhysicsObj &obj2, float restitution)
		{
			agl::Vec<float, 2> obj1PerpRadius = {-obj1Inter.y, obj1Inter.x};
			agl::Vec<float, 2> obj2PerpRadius = {-obj2Inter.y, obj2Inter.x};

			agl::Vec<float, 2> relVel = (obj2.velocity + (obj2PerpRadius * -obj2.angularVelocity)) -
										(obj1.velocity + (obj1PerpRadius * -obj1.angularVelocity));

			float impulse = (relVel * -(1 + restitution)).dot(normal);

			float add1 = std::pow(obj1PerpRadius.dot(normal), 2) * obj1.invInertia;
			float add2 = std::pow(obj2PerpRadius.dot(normal), 2) * obj2.invInertia;

			impulse /= ((obj1.invMass + obj2.invMass)) + add1 + add2;

			obj1.force -= normal * impulse;
			obj2.force += normal * impulse;

			obj1.angularForce += obj1PerpRadius.dot(normal * impulse);
			obj2.angularForce -= obj2PerpRadius.dot(normal * impulse);

			obj1.posOffset += normal * overlap * (obj1.mass / (obj1.mass + obj2.mass));
			obj2.posOffset -= normal * overlap * (obj2.mass / (obj1.mass + obj2.mass));
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
				return a;
			}
			else if (d >= 1)
			{
				return b;
			}
			else
			{
				return a + ab * d;
			}
		}
};
#endif
