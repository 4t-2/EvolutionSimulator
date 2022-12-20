#include "../inc/other.hpp"
#include <math.h>

float loop(float min, float max, float value)
{
	return value - (max + abs(min)) * int(value / max);
}

float vectorAngle(agl::Vec<float, 2> vec)
{
	float angle = atan(vec.x / vec.y);

	if (vec.y < 0)
	{
		angle *= -1;

		if (vec.x > 0)
		{
			angle = PI - angle;
		}
		else
		{
			angle = -(PI + angle);
		}
	}

	return angle;
}

