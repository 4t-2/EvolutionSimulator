#pragma once

#include "PhysicsObj.hpp"

class ViteGenome
{
	public:
		agl::Vec<float, 2> size;
		float			   armour;
		float			   damage;
		float			   muscle;
		int				   hue;
};

class ViteSeg : public PhysicsObj
{
	public:
		float health;
		float energy;

		int						geneIndex;
		std::vector<ViteGenome> genome;

		ViteSeg				  *parent;
		std::vector<ViteSeg *> children;

		void setup(agl::Vec<float, 2> pos = {0, 0})
		{
			PhysicsObj::setup(pos, genome[geneIndex].size,
							  (genome[geneIndex].size.x * genome[geneIndex].size.y) / 100.);

			if(parent != nullptr)
			{
				position.y = parent->position.y + parent->size.y / 2 + size.y / 2;
				position.x = parent->position.x;
			}
		}
};
