#pragma once
#include <AGL/agl.hpp>

float loop(float min, float max, float value);

float vectorAngle(agl::Vec<float, 2> vec);

agl::Color hueToRGB(int hue);

int roundUp(float input, int period);

struct Debug
{
		static std::vector<std::string> log;
};

float cross2D(agl::Vec<float, 2> a, agl::Vec<float, 2> b);

agl::Vec<float, 2> perp(agl::Vec<float, 2> vec);

agl::Vec<float, 2> closestPointToLine(agl::Vec<float, 2> a, agl::Vec<float, 2> b, agl::Vec<float, 2> p);
