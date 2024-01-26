#pragma once

#include "AGL/include/math.hpp"
#include "Environment.hpp"
#include "IN/inc/NeuralNetwork.hpp"
#include "PhysicsObj.hpp"
#include <iterator>

struct RectDef
{
		agl::Vec<float, 2> size;
		agl::Vec<float, 2> pos;
		float			   rot;
		agl::Color		   col;
};

struct JointDef
{
		int				   start;
		int				   end;
		agl::Vec<float, 2> local1;
		agl::Vec<float, 2> local2;
};

class NewCreature
{
	public:
		std::vector<RectDef>  rectDefs;
		std::vector<JointDef> jointDefs;

		std::vector<PhyRect *> rect;
		std::vector<PhyJoint>  joint;

		in::NeuralNetwork *network = nullptr;

		PhyRect		   *selected		= nullptr;
		int				indexOfSelected = -1;
		static PhyRect *grund;

		static World *world;
		static Environment *env;

		static float torque;
		static int	 brainMutations;

		int id = 0;

		int frame = 0;

		bool taintedNetwork = true;

		NewCreature(int id)
		{
			this->id = id;
		}

		~NewCreature()
		{
			if (network != nullptr)
			{
				network->destroy();
				delete network;
			}
			clear();
		}

		void selectRect(agl::Vec<float, 2> pos)
		{
			for (PhyRect *&o : rect)
			{
                World::PolyShape s;
                world->boxToPoly(*o->phyBody, s);
				bool intersect = world->pointInBox<false>(s, pos/ SIMSCALE);

				if (intersect)
				{
					unselect();
					selected		= o;
					selected->color = agl::Color::Red;
					indexOfSelected = ((long long)&o - (long long)(&*rect.begin())) / 8;
				}
			}
		}

		void unselect()
		{
			if (selected != nullptr)
			{
				selected->color = selected->realColor;
				selected		= nullptr;
				indexOfSelected = -1;
			}
		}

		void deletePart(PhyRect *phy)
		{
			if (phy == rect[0] || phy == nullptr)
			{
				return;
			}

			taintedNetwork = true;

			for (auto it = joint.begin(); it != joint.end(); it++)
			{
				PhyJoint &j = *it;

				if (j.start == phy || j.end == phy)
				{
					auto s = j.start;

					// world->DestroyJoint(j.joint);

					it--;
					joint.erase(std::next(it, 1));

					if (j.start == phy)
					{
						deletePart(j.end);
					}
				}
			}

			for (auto it = rect.begin(); it != rect.end(); it++)
			{
				PhyRect *&r = *it;

				if (r == phy)
				{
					world->DestroyBody(r->phyBody);

					int i = it - rect.begin();

					it--;
					rect.erase(std::next(it, 1));

					rectDefs.erase(std::next(rectDefs.begin(), i));

					break;
				}
			}
		}

		void createPart(agl::Vec<float, 2> size, agl::Vec<float, 2> pos, float rotation, agl::Vec<float, 2> globalStart)
		{
			taintedNetwork = true;

			PhyRect &r = env->addEntity<PhyRect>();

			r.setup(size, pos, rotation, *world, b2_dynamicBody, id);
			r.realColor = {(unsigned char)(255 * ((float)rand() / (float)RAND_MAX)),
						   (unsigned char)(255 * ((float)rand() / (float)RAND_MAX)),
						   (unsigned char)(255 * ((float)rand() / (float)RAND_MAX))};
			r.color		= r.realColor;
            r.phyBody->forcable = true;

			rectDefs.push_back({size, pos, rotation, r.realColor});

			rect.emplace_back(&r);
return;
			agl::Vec<float, 2> lcoal1 = globalStart - selected->position;

			agl::Mat4f rot;
			rot.rotateZ(agl::radianToDegree(-selected->rotation));

			lcoal1 = rot * lcoal1;

			joint.emplace_back();
			joint[joint.size() - 1].setup(*selected, r, lcoal1, {0, size.y / -2}, *world);

			jointDefs.push_back({indexOfSelected,
								 static_cast<int>((long long)(&rect.back()) - (long long)(&*rect.begin())) / 8,
								 lcoal1,
								 {0, size.y / -2}});
		}

		bool touchingSelected(agl::Vec<float, 2> pos)
		{
			if (selected != nullptr)
			{
                World::PolyShape s;
                world->boxToPoly(*selected->phyBody, s);
				bool intersect = world->pointInBox<false>(s, pos/ SIMSCALE);
				return intersect;
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
			rectDefs.clear();
			selected = nullptr;
		}

		void def()
		{
			auto &a = env->addEntity<PhyRect>();
			a.setup({30, 30}, {0, 0}, PI / 3, *world, b2_dynamicBody, id);

			rectDefs.push_back({{30, 30}, {0, 0}, PI / 3, agl::Color::White});
			rect.emplace_back(&a);

			taintedNetwork = true;
		}

		void clone(NewCreature &creature)
		{
			taintedNetwork = true;

			for (auto &r : creature.rectDefs)
			{
				auto &o = env->addEntity<PhyRect>();
				rect.emplace_back(&o);
				rectDefs.emplace_back(r);

				o.setup(r.size, r.pos, r.rot, *world, b2_dynamicBody, id);
				o.color = r.col;
			}

			for (auto &j : creature.jointDefs)
			{
				joint.emplace_back();

				joint[joint.size() - 1].setup(*rect[j.start], *rect[j.end], j.local1, j.local2, *world);
				jointDefs.push_back(j);
			}
		}

		void setupNetwork()
		{
			if (!taintedNetwork)
			{
				network->setActivation(in::tanh);
				return;
			}

			in::NetworkStructure ns((joint.size() * 2) + rect.size() + 2 + 2, {}, joint.size(), true);

			in::NetworkStructure::randomWeights(ns);

			network = new in::NeuralNetwork(ns);

			// for each joint get rotation, speed
			// for each part, grounbd collision
			// for just head, height off ground and rotation
			//
			// output for each joint choose motor speed / direction

			network->setActivation(in::ActivationFunction::tanh);
		}

		void updateNetwork()
		{
			int node = 2;

			network->setInputNode(0, 1);
			network->setInputNode(1, sin(frame / 20.));

			for (int i = 0; i < joint.size(); i++)
			{
				network->setInputNode(node, joint[i].joint->GetJointAngle() / (PI / 2));
				node++;
				network->setInputNode(node, joint[i].joint->GetMotorSpeed() / (PI / 2));
				node++;
			}

			for (int i = 0; i < rect.size(); i++)
			{
				// for (b2ContactEdge *ce = rect[i]->phyBody->GetContactList(); ce; ce = ce->next)
				// {
				// 	b2Contact *c = ce->contact;
				//
				// 	if (grund != nullptr)
				// 	{
				// 		if (ce->other == grund->phyBody)
				// 		{
				// 			network->setInputNode(node, 1);
				// 			break;
				// 		}
				// 		else
				// 		{
				// 			network->setInputNode(node, 0);
				// 		}
				// 	}
				// 	else
				// 	{
				// 		network->setInputNode(node, 0);
				// 	}
				// }

				node++;
			}

			network->setInputNode(node, (rect[0]->position.y + 100) / 150.);
			node++;
			network->setInputNode(node, -rect[0]->rotation / (2 * PI));

			network->update();

			for (int i = 0; i < network->structure.totalOutputNodes; i++)
			{
				// {
				// 	joint[i].joint->SetMotorSpeed(network->outputNode[i].value / 6);
				// }

				{
					float ang = joint[i].joint->GetJointAngle();
					float net = network->outputNode[i].value * (PI / 2);

					net = sin(frame / 10.);

					joint[i].joint->SetMotorSpeed((1. / 6) * (net - ang));
				}
			}

			frame++;
		}

		void clearNetwork()
		{
			taintedNetwork = true;

			if (network != nullptr)
			{
				delete network;
				network = nullptr;
			}
		}

		void setNetwork(in::NetworkStructure &ns)
		{
			taintedNetwork = false;

			network = new in::NeuralNetwork(ns);
			network->setActivation(in::tanh);
		}

		void cloneMutateNetwork(NewCreature &creature)
		{
			taintedNetwork = false;

			in::NetworkStructure ns = creature.network->structure;

			for (int i = 0; i < brainMutations; i++)
			{
				ns.mutate();
			}

			network = new in::NeuralNetwork(ns);
			network->setActivation(in::tanh);
		}

		void clonePerfectNetwork(NewCreature &creature)
		{
			taintedNetwork = false;

			in::NetworkStructure ns = creature.network->structure;

			network = new in::NeuralNetwork(ns);
			network->setActivation(in::tanh);
		}
};
