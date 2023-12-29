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
