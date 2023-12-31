#pragma once

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

		NewCreature()
		{
			auto &a = env->addEntity<PhyRect>();
			a.setup({20, 20}, {0, 0}, 0, *world, b2_dynamicBody);

			rect.emplace_back(&a);
		}

		void selectRect(agl::Vec<float, 2> pos)
		{
			for (PhyRect *&o : rect)
			{
				bool intersect = o->phyBody->GetFixtureList()[0].TestPoint(PhysicsObj::scalePos(pos));

				std::cout << '\n';
				std::cout << intersect << '\n';
				std::cout << pos << '\n';

				if (intersect)
				{
					if (selected != nullptr)
					{
						selected->color = agl::Color::White;
					}
					selected		= o;
					selected->color = agl::Color::Red;
				}
			}
		}

		void createPart(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, float rotation, agl::Vec<float, 2> globalStart)
		{
			PhyRect &r = env->addEntity<PhyRect>();

			r.setup(size, pos, rotation, *world, b2_dynamicBody);

			rect.emplace_back(&r);

			joint.emplace_back();
			joint[joint.size() - 1].setup(*selected, r, globalStart - selected->position, {0, size.y / -2}, *world);
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
};
