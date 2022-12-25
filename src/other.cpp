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

agl::Color hueToRGB(int hue)
{
	hue = hue % 360;
	agl::Color color;

	if (hue < 60)
	{
		color.r = 255;
		color.g = float(hue - 0) / 60 * 255;
		color.b = 0;
	}
	else if (hue < 120)
	{
		color.r = 255 - (float(hue - 60) / 60 * 255);
		color.g = 255;
		color.b = 0;
	}
	else if (hue < 180)
	{
		color.r = 0;
		color.g = 255;
		color.b = float(hue - 120) / 60 * 255;
	}
	else if (hue < 240)
	{
		color.r = 0;
		color.g = 255 - (float(hue - 180) / 60 * 255);
		color.b = 255;
	}
	else if (hue < 300)
	{
		color.r = float(hue - 240) / 60 * 255;
		color.g = 0;
		color.b = 255;
	}
	else if (hue < 360)
	{
		color.r = 255;
		color.g = 0;
		color.b = 255 - (float(hue - 300) / 60 * 255);
	}

	return color;
}
