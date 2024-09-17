#pragma once

#include "AGL/include/ShaderBuilder.hpp"
#include "other.hpp"
#include <AGL/agl.hpp>
#include <algorithm>

struct PolyShape
{
		std::vector<agl::Vec<float, 2>> points;
		std::vector<agl::Vec<float, 2>> normals;
};

class PhysicsObj
{
	public:
		bool			   exists;
		agl::Vec<float, 2> position;
		int				   collideCount		   = 0;
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
		float			   oldImpulse = 0;

		PhysicsObj		  *rootConnect = nullptr;
		agl::Vec<float, 2> local1;
		agl::Vec<float, 2> local2;
		float			   maxMotor = 1;

		PolyShape *shape;

		PhysicsObj()
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
			inertia = (1 / 12.f) * mass * (size.x * size.x + size.y * size.y);
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
			return loop((float)-PI, (float)PI, (rotation - rootConnect->rotation) - refRot);
		}

		static float sec(float x)
		{
			return 1. / cos(x);
		}
		static float csc(float x)
		{
			return 1. / sin(x);
		}

		static float sq(float x)
		{
			return x * x;
		}

		// depth, norm
		inline std::pair<float, agl::Vec<float, 2>> getDistIfCol(agl::Vec<float, 2> pos)
		{
			agl::Vec<float, 2> offset = position - pos;

			float rot = offset.angle();

			float regDist = offset.dot(offset);

			float sqDist = fmin(sq(size.x / 2 * sec(rot - rotation)), sq(size.y / 2 * csc(rot - rotation)));

			if (regDist > sqDist)
			{
				return std::pair(0, agl::Vec<float, 2>{0, 0});
			}
			else
			{
				float depth	 = sqrt(sqDist) - sqrt(regDist);
				float fixRot = ((rot + PI / 4) > 2 * PI ? rot + PI / 4 - (2 * PI) : rot + PI / 4);
				std::cout << fixRot << '\n';
				float normRot = int(fixRot / (PI / 2)) * (PI / 2) + rotation;

				return std::pair(depth, agl::Vec<float, 2>{-sin(normRot), cos(normRot)});
			}
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
		agl::Vec<float, 2> normal = {0, 0};
		float			   depth  = 0;
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
			float vel	  = b1.rootConnect->angularVelocity - b1.angularVelocity;
			float impulse = (b1.motor - vel); //* (1 / (b1.inertia + b1.rootConnect->inertia));

			// float torque= 1;
			// impulse = std::clamp(impulse, -torque, torque);

			float oldImp  = b1.oldImpulse;
			b1.oldImpulse = oldImp;
			impulse		  = impulse - oldImp;

			// float impulse = b1.motor;

			b1.angularAcceleration -= impulse * b1.invInertia;
			b1.rootConnect->angularAcceleration += impulse * b1.rootConnect->invInertia;

			// std::cout << impulse << '\n';

			// float torque = 100000000;
			//
			// float Cdot		   = b1.angularVelocity -
			// b1.rootConnect->angularVelocity
			// - b1.motor; float impulse	   = (1 / (b1.rootConnect->inertia +
			// b1.inertia)
			// * Cdot); float motorImpulse = std::clamp(b1.oldImpulse + impulse,
			// -torque, torque);
			//          std::cout << motorImpulse << '\n';
			// impulse			   = motorImpulse;
			//          b1.oldImpulse = motorImpulse;
			//
			// b1.angularAcceleration -= impulse * b1.invInertia;
			// b1.rootConnect->angularAcceleration += impulse *
			// b1.rootConnect->invInertia;
		}

		template <bool useBare = true>
		static bool pointInBox(PolyShape &shape, agl::Vec<float, 2> point, ConstraintFailure &bare)
		{
			float			   depth = INFINITY;
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

		static void resolve(agl::Vec<float, 2> input1, agl::Vec<float, 2> input2, float rot1, float rot2,
							float invInertia1, float invInertia2, float invMass1, float invMass2,
							agl::Vec<float, 2> normal, agl::Vec<float, 2> r1, agl::Vec<float, 2> r2,
							agl::Vec<float, 2> &out1, agl ::Vec<float, 2> &out2, float &rotOut1, float &rotOut2,
							int divider)
		{
			agl::Vec<float, 2> rp1	  = perp(r1);
			agl::Vec<float, 2> rp2	  = perp(r2);
			agl::Vec<float, 2> relVel = (input1 + (rp1 * -rot1)) - (input2 + (rp2 * -rot2));

			float restitution = 1;
			auto  top		  = (relVel * -(1 + restitution)).dot(normal);

			float botl1 = std::pow(rp1.dot(normal), 2) * invInertia1;
			float botl2 = std::pow(rp2.dot(normal), 2) * invInertia2;

			auto  bottom  = normal.dot(normal * (invMass1 + invMass2)) + botl1 + botl2;
			float impulse = top / bottom;

			impulse /= divider;

			agl::Vec<float, 2> acc1 = normal * (impulse * invMass1);
			agl::Vec<float, 2> acc2 = normal * (impulse * -invMass2);

			out1 += acc1;
			out2 += acc2;

			rotOut1 -= rp1.dot(normal * impulse) * invInertia1;
			rotOut2 += rp2.dot(normal * impulse) * invInertia2;
		}
};

class TestObj : public PhysicsObj
{
	public:
		TestObj() : PhysicsObj()
		{
			return;
		}
};

class CollisionConstraint
{
	public:
		static void probe(PhysicsObj &b1, PhysicsObj &b2, std::vector<ConstraintFailure> &failure)
		{
			PolyShape s1;
			PolyShape s2;
			// CHECK COLLISION
			b1.boxToPoly(s1);
			b2.boxToPoly(s2);

			auto compCol = [&](PhysicsObj &a, PhysicsObj &b, PolyShape &sa, PolyShape &sb) {
				for (agl::Vec<float, 2> &point : sb.points)
				{
					ConstraintFailure bare;
					if (World::pointInBox(sa, point, bare))
					{
						failure.emplace_back(bare);
						failure.back().b1 = &a;
						failure.back().b2 = &b;
						failure.back().r1 = failure.back().b1->position - failure.back().inter1;
						failure.back().r2 = failure.back().b2->position - failure.back().inter2;

						failure.back().b1->collideCount++;
						failure.back().b2->collideCount++;
					}

					/*
										auto res = a.getDistIfCol(point);

										if (res.first > 0)
										{
												failure.push_back({});
												failure.back().b1	  = &a;
												failure.back().b2	  = &b;
												failure.back().inter1 = point + (res.second
					   * res.first); failure.back().inter2 = point; failure.back().normal =
					   res.second; failure.back().depth  = res.first; failure.back().r1
					   = failure.back().b1->position - failure.back().inter1;
												failure.back().r2	  =
					   failure.back().b2->position - failure.back().inter2;

												failure.back().b1->collideCount++;
												failure.back().b2->collideCount++;
										}
					 */
				}
			};

			compCol(b1, b2, s1, s2);
			compCol(b2, b1, s2, s1);
		}
};

class JointConstraint
{
	public:
		static void probe(PhysicsObj &b1, PhysicsObj &b2, std::vector<ConstraintFailure> &failure)
		{
			agl::Mat4f rot;
			rot.rotateZ(-b1.radToDeg());

			agl::Vec<float, 2> local1 = b1.position + (rot * b1.local1);
			rot.rotateZ(-b1.rootConnect->radToDeg());
			agl::Vec<float, 2> local2 = b1.rootConnect->position + (rot * b1.local2);

			agl::Vec<float, 2> offset = local1 - local2;
			agl::Vec<float, 2> normal = offset.normalized();
			float			   dist	  = offset.length();

			if (dist == 0)
			{
				return;
			}

			failure.emplace_back();

			failure.back().b1	  = &b1;
			failure.back().b2	  = b1.rootConnect;
			failure.back().normal = normal;
			failure.back().depth  = dist;
			failure.back().inter1 = local1;
			failure.back().inter2 = local2;
			failure.back().r1	  = b1.position - failure.back().inter1;
			failure.back().r2	  = b1.rootConnect->position - failure.back().inter2;

			failure.back().b1->collideCount++;
			failure.back().b2->collideCount++;
		}
};
