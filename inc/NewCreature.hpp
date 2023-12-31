#pragma once

#include "AGL/include/math.hpp"
#include "Environment.hpp"
#include "PhysicsObj.hpp"

class NewCreature
{
	public:
		std::vector<PhyRect *> rect;
		std::vector<PhyJoint>  joint;

		PhyRect *selected = nullptr;

		static b2World	   *world;
		static Environment *env;

		~NewCreature()
		{
			clear();
		}

		void selectRect(agl::Vec<float, 2> pos)
		{
			for (PhyRect *&o : rect)
			{
				bool intersect = o->phyBody->GetFixtureList()[0].TestPoint(PhysicsObj::scalePos(pos));

				if (intersect)
				{
					unselect();
					selected		= o;
					selected->color = agl::Color::Red;
				}
			}
		}

		void unselect()
		{
			if (selected != nullptr)
			{
				selected->color = selected->realColor;
				selected		= nullptr;
			}
		}

		void createPart(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, float rotation, agl::Vec<float, 2> globalStart)
		{
			PhyRect &r = env->addEntity<PhyRect>();

			r.setup(size, pos, rotation, *world, b2_dynamicBody);
			r.realColor = {(unsigned char)(255 * ((float)rand() / (float)RAND_MAX)),
						   (unsigned char)(255 * ((float)rand() / (float)RAND_MAX)),
						   (unsigned char)(255 * ((float)rand() / (float)RAND_MAX))};
			r.color		= r.realColor;

			rect.emplace_back(&r);

			agl::Vec<float, 2> lcoal1 = globalStart - selected->position;

			agl::Mat4f rot;
			rot.rotateZ(agl::radianToDegree(-selected->rotation));

			lcoal1 = rot * lcoal1;

			joint.emplace_back();
			joint[joint.size() - 1].setup(*selected, r, lcoal1, {0, size.y / -2}, *world);
		}

		bool touchingSelected(agl::Vec<float, 2> pos)
		{
			if (selected != nullptr)
			{
				return selected->phyBody->GetFixtureList()[0].TestPoint(PhysicsObj::scalePos(pos));
			}
			else
			{
				return false;
			}
		}

		void compile()
		{
		}

		void clear()
		{
			rect.clear();
			joint.clear();
			selected = nullptr;
		}

		void def()
		{
			auto &a = env->addEntity<PhyRect>();
			a.setup({30, 30}, {0, 0}, 0, *world, b2_dynamicBody);

			rect.emplace_back(&a);
		}

		void clone(NewCreature &creature)
		{
			for (auto r : creature.rect)
			{
				auto &o = env->addEntity<PhyRect>();
				rect.emplace_back(&o);

				o.setup(r->size, r->position, r->rotation, *world, b2_dynamicBody);
				o.color = r->color;
			}

			for (auto &j : creature.joint)
			{
				int first;
				int second;

				for (int i = 0; i < creature.rect.size(); i++)
				{
					if (creature.rect[i] == j.start)
					{
						first = i;
					}

					if (creature.rect[i] == j.end)
					{
						second = i;
					}
				}

				joint.emplace_back();
				joint[joint.size() - 1].setup(*rect[first], *rect[second], j.local1, j.local2, *world, 25);
			}
		}
};
