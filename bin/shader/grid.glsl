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

	const int gridSize = 5;

	if(mod(fragPos.x+(.5*scale), gridSize) < (1*scale) || mod(fragPos.y+(.5*scale), gridSize) < (1*scale))
	{
	    color = vec4(0, 0, 0, 1);
	}
}
