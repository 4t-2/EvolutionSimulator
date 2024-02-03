#include "../inc/other.hpp"
#include <math.h>

std::vector<std::string> Debug::log;

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

int roundUp(float input, int period)
{
	float amount = input / period;
	amount += (amount - (int)amount) > 0;

	return (int)amount * period;
}

float cross2D(agl::Vec<float, 2> a, agl::Vec<float, 2> b)
{
	return a.x * b.y - a.y * b.x;
}

agl::Vec<float, 2> perp(agl::Vec<float, 2> vec)
{
	return {-vec.y, vec.x};
}

agl::Vec<float, 2> closestPointToLine(agl::Vec<float, 2> a, agl::Vec<float, 2> b, agl::Vec<float, 2> p)
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
