#version 330 core
#define PI 3.1415926535897932384626433832795

in vec2 UVcoord;
in vec4 fragPos;
in vec2 scale;
in vec4 fragColor;

out vec4 color;

uniform sampler2D textureSampler;

void main()
{
	vec2 pixSize = scale / 2;

	if(fragPos.x <= 1 || fragPos.x >= (scale.x - 1) || fragPos.y <= 1 || fragPos.y >= (scale.y-1))
	{
		if((fragPos.x <= 1 && fragPos.y <= 1) || (fragPos.x <= 1 && fragPos.y >= (scale.y-1)) || (fragPos.x >= (scale.x-1) && fragPos.y <= 1) || (fragPos.x >= (scale.x-1) && fragPos.y >= (scale.y-1)))
		{
			color = vec4(0, 0, 0, 0);
		} else
		{
			color = vec4(92./255, 92./255, 92./255, 1);
		}
	} else
	{
		float start = 130;
		float end = 200;

		vec2 sin = sin((floor(UVcoord * pixSize) * pixSize/(pixSize-1)) / pixSize * PI);
		float val = sin.x < sin.y ? sin.x : sin.y;

		val = (val * (end - start) + start) / 255;

		color = vec4(val, val, val, 1);
	}

	color *= fragColor;
}
