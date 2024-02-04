#pragma once

#include "Environment.hpp"
#include "other.hpp"
#include <AGL/agl.hpp>

struct PolyShape
{
		std::vector<agl::Vec<float, 2>> points;
		std::vector<agl::Vec<float, 2>> normals;
};

class PhysicsObj : public BaseEntity
{
	public:
		int				   id				   = 0;
		bool			   forcable			   = false;
		bool			   dynamic			   = false;
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
		float			   rotOffset		   = 0;
		float			   refRot			   = 0;
		float			   motor			   = 0;
		agl::Vec<float, 2> force;

		PhysicsObj		  *rootConnect = nullptr;
		agl::Vec<float, 2> local1;
		agl::Vec<float, 2> local2;

		PolyShape *shape;

		PhysicsObj(bool &exists, agl::Vec<float, 2> &position) : BaseEntity(exists, position)
		{
		}

		void setup(agl::Vec<float, 2> position, agl::Vec<float, 2> size, float mass)
		{
			this->position = position;
			this->size	   = size;
			setMass(mass);
		}

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

		void syncOffsets()
		{
			position += posOffset;
			rotation += rotOffset;

			posOffset = {0, 0};
			rotOffset = 0;
		}

		void updatePhysics()
		{
			velocity			= velocity + acceleration;
			angularVelocity		= angularVelocity + angularAcceleration;
			acceleration		= {0, 0};
			angularAcceleration = 0;

			position += velocity;
			rotation += angularVelocity;

			syncOffsets();

			// rotation = loop(-PI, PI, rotation);
		}

		void SetTransform(agl::Vec<float, 2> pos, float rot)
		{
			position = pos;
			rotation = rot;
		}

		void ApplyForceToCenter(agl::Vec<float, 2> force)
		{
			acceleration += force * invMass;
		}

		void applyForce(PhysicsObj &b, agl::Vec<float, 2> force, agl::Vec<float, 2> point)
		{
			b.acceleration += force * b.invMass;
			b.angularAcceleration += cross2D(point - b.position, force) * b.invInertia;
		}

		static void addJoint(PhysicsObj &b1, agl::Vec<float, 2> local1, PhysicsObj &b2, agl::Vec<float, 2> local2)
		{
			b1.rootConnect = &b2;
			b1.local1	   = local1;
			b1.local2	   = local2;
			b1.refRot	   = b1.rotation - b2.rotation;
		}

		void boxToPoly(PolyShape &shape)
		{
			agl::Mat4f rot;
			rot.rotateZ(-radToDeg());

			shape.points.resize(4);
			shape.points[0] = position + rot * agl::Vec<float, 2>{-size.x, -size.y} / 2;
			shape.points[1] = position + rot * agl::Vec<float, 2>{size.x, -size.y} / 2;
			shape.points[2] = position + rot * agl::Vec<float, 2>{size.x, size.y} / 2;
			shape.points[3] = position + rot * agl::Vec<float, 2>{-size.x, size.y} / 2;

			shape.normals.resize(4);
			shape.normals[0] = rot * agl::Vec<float, 2>{0, -1};
			shape.normals[1] = rot * agl::Vec<float, 2>{1, 0};
			shape.normals[2] = rot * agl::Vec<float, 2>{0, 1};
			shape.normals[3] = rot * agl::Vec<float, 2>{-1, 0};

			this->shape = &shape;
		}

		float getJointAngle()
		{
			return loop(-PI, PI, (rotation - rootConnect->rotation) - refRot);
		}
};

struct ConstraintFailure
{
		PhysicsObj		  *b1;
		PhysicsObj		  *b2;
		agl::Vec<float, 2> inter1 = {0, 0};
		agl::Vec<float, 2> inter2 = {0, 0};
		agl::Vec<float, 2> r1;
		agl::Vec<float, 2> r2;
		agl::Vec<float, 2> normal  = {0, 0};
		float			   depth   = 0;
		bool			   occured = false;
};

class CollisionConstraint
{
	public:
		static ConstraintFailure probe(PhysicsObj &, PhysicsObj &)
		{
		}

		static void solve()
		{
		}
};

class World
{
	public:
		static agl::Vec<float, 2> AABBnormal(PhysicsObj &box1, PhysicsObj &box2)
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

		static void motor(PhysicsObj &b1)
		{
			if (b1.getJointAngle() > (PI / 4))
			{
				b1.motor = .1;
			}
			if (b1.getJointAngle() < -(PI / 4))
			{
				b1.motor = -.1;
			}

            float maxImpulse = .015;

			float vel	  = b1.rootConnect->angularVelocity - b1.angularVelocity;
			float impulse = b1.motor - vel;

            // float impulse = b1.motor;

			b1.angularVelocity -= impulse * b1.invMass;
			b1.rootConnect->angularVelocity += impulse * b1.rootConnect->invInertia;
		}

		static void processJoint(PhysicsObj &b1, ConstraintFailure &bare)
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

			motor(b1);
		}

		template <bool useBare = true>
		static bool pointInBox(PolyShape &shape, agl::Vec<float, 2> point,
							   ConstraintFailure &bare = *(ConstraintFailure *)nullptr)
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
						bare.inter1 = online;
						bare.inter2 = point;
						bare.normal = shape.normals[index];
						bare.depth	= depth;
					}
					return true;
				}
			}

			return false;
		}

		static void testRes(ConstraintFailure &collision, int divider)
		{
			agl::Vec<float, 2> rp1	  = perp(collision.r1);
			agl::Vec<float, 2> rp2	  = perp(collision.r2);
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

			collision.b1->rotOffset -= rp1.dot(collision.normal * impulse) * collision.b1->invInertia;
			collision.b2->rotOffset += rp2.dot(collision.normal * impulse) * collision.b2->invInertia;
		}

		static void resolve(ConstraintFailure &collision, int divider)
		{
			agl::Vec<float, 2> rp1	  = perp(collision.r1);
			agl::Vec<float, 2> rp2	  = perp(collision.r2);
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
		}

		static void collide(PhysicsObj &b1, PhysicsObj &b2)
		{
			PolyShape s1;
			PolyShape s2;
			// CHECK COLLISION
			b1.boxToPoly(s1);
			b2.boxToPoly(s2);

			std::vector<ConstraintFailure> collision;

			auto compCol = [&](PhysicsObj &a, PhysicsObj &b, PolyShape &sa, PolyShape &sb) {
				for (agl::Vec<float, 2> &point : sb.points)
				{
					ConstraintFailure bare;
					if (pointInBox(sa, point, bare))
					{
						collision.emplace_back(bare);
						collision.back().b1 = &a;
						collision.back().b2 = &b;
						collision.back().r1 = collision.back().b1->position - collision.back().inter1;
						collision.back().r2 = collision.back().b2->position - collision.back().inter2;
					}
				}
			};

			if (b1.rootConnect == &b2)
			{
				ConstraintFailure bare;
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
					collision.back().r1		= collision.back().b1->position - collision.back().inter1;
					collision.back().r2		= collision.back().b2->position - collision.back().inter2;
				}
			}
			else if (b2.rootConnect == &b1)
			{
				ConstraintFailure bare;
				processJoint(b2, bare);

				if (bare.depth != 0)
				{
					collision.emplace_back();
					collision.back().b1		= &b2;
					collision.back().b2		= &b1;
					collision.back().normal = bare.normal;
					collision.back().inter1 = bare.inter1;
					collision.back().inter2 = bare.inter2;
					collision.back().depth	= bare.depth;
					collision.back().r1		= collision.back().b1->position - collision.back().inter1;
					collision.back().r2		= collision.back().b2->position - collision.back().inter2;
				}
			}
			else
			{
				compCol(b1, b2, s1, s2);
				compCol(b2, b1, s2, s1);
			}

			for (ConstraintFailure &c : collision)
			{
				resolve(c, collision.size());
			}
		}
};

class TestObj : public Entity<PhysicsObj>
{
	public:
		bool			   exists = true;
		agl::Vec<float, 2> position;

		TestObj() : Entity<PhysicsObj>(exists, position)
		{
			return;
		}
};
