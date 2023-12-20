#version 330 core

in vec2 UVcoord;
in vec4 fragColor;
in vec4 fragPos;

out vec4 color;

uniform float scale;
uniform sampler2D textureSampler;

void main()
{
    color = fragColor;

	const int gridSize = 100;

	if(mod(fragPos.x+(.5*scale), 1280) < (1*scale) || mod(fragPos.y+(.5*scale), 720) < (1*scale))
	{
	    color = vec4(1, 1, 1, 1);
	}
}
