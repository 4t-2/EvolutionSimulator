#version 330 core
#define PI 3.1415926535897932384626433832795

in vec2 UVcoord;
in vec4 fragPos;
in vec2 scale;
in vec4 fragColor;

out vec4 color;

uniform sampler2D textureSampler;
uniform float isJoint;
uniform float time;

void main()
{
	vec2 pixSize = scale / 2;
	if(isJoint < .5)
	{
		if(fragPos.x <= 1 || fragPos.x >= (scale.x - 1) || fragPos.y <= 1 || fragPos.y >= (scale.y-1))
		{
			color = vec4(192./255, 192./255, 192./255, 1) * fragColor;
		} else
		{
			float start = 130;
			float end = (100 * (sin(time / 30) / 2 + .5)) + 155;

			vec2 sin = sin((floor(UVcoord * pixSize) * pixSize/(pixSize-1)) / pixSize * PI);
			float val = sin.x < sin.y ? sin.x : sin.y;

			val = (val * (end - start) + start) / 255;

			color = vec4(fragColor.xyz * val, fragColor.w);
		}
	} else
	{
		color = vec4(192./255, 192./255, 192./255, 1) * fragColor;
	}
}
