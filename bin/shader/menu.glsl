#version 330 core
#define PI 3.1415926535897932384626433832795

in vec2 UVcoord;
in vec4 fragPos;
in vec2 scale;
in vec4 fragColor;
flat in int typeM;

out vec4 color;

uniform sampler2D textureSampler;

void main()
{
	if(typeM == 0)
	{
		if(fragPos.x <= 1 || fragPos.y <= 1)
		{
			color = vec4(1, 1, 1, 1);
		} else if(fragPos.x >= (scale.x - 1) || fragPos.y >= (scale.y - 1))
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		}
	} else if(typeM == 1)
	{
		if(fragPos.x >= (scale.x - 1) || fragPos.y >= (scale.y - 1))
		{
			color = vec4(0, 0, 0, 1);
		} else if(fragPos.x <= 1 || fragPos.y <= 1)
		{
			color = vec4(1, 1, 1, 1);
		} else if(fragPos.x >= (scale.x - 2) || fragPos.y >= (scale.y - 2))
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else if(fragPos.x <= 2 || fragPos.y <= 2)
		{
			color = vec4(223./255, 223./255, 223./255, 1);
		} else
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		}
	} else if(typeM == 2)
	{
		if(fragPos.x <= 1 || fragPos.y <= 1)
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else if(fragPos.x >= (scale.x - 1) || fragPos.y >= (scale.y - 1))
		{
			color = vec4(1, 1, 1, 1);
		} else
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		}
	} else if(typeM == 3)
	{
		if(fragPos.x <= 1 || fragPos.y <= 1)
		{
			color = vec4(0, 0, 0, 1);
		} else if(fragPos.x >= (scale.x - 1) || fragPos.y >= (scale.y - 1))
		{
			color = vec4(1, 1, 1, 1);
		} else if(fragPos.x <= 2 || fragPos.y <= 2)
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else if(fragPos.x >= (scale.x - 2) || fragPos.y >= (scale.y - 2))
		{
			color = vec4(223./255, 223./255, 223./255, 1);
		} else
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		}
	} else if(typeM == 4)
	{
		if((fragPos.x >= (scale.x-1) && fragPos.y <= 1) || (fragPos.x <= 1 && fragPos.y >= (scale.y-1)))
		{
			color = vec4(0, 0, 0, 0);
		} else if(fragPos.x <= 1 || fragPos.y <= 1)
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else if(fragPos.x >= (scale.x - 1) || fragPos.y >= (scale.y - 1))
		{
			color = vec4(1, 1, 1, 1);
		} else if(fragPos.y <= 2)
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		} else if(fragPos.y >= (scale.y - 2))
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		} else
		{
			color = vec4(1, 1, 1, 1);
		}
	} else if(typeM == 5)
	{
		if(fragPos.x <= 1 || fragPos.x >= (scale.x - 1) || fragPos.y <= 1 || fragPos.y >= (scale.y-1))
		{
			color = vec4(0, 0, 0, 1);
		} else if(fragPos.x <= 2 || fragPos.x >= (scale.x - 2) || fragPos.y <= 2 || fragPos.y >= (scale.y-2))
		{
			color = vec4(128./255, 128./255, 128./255, 1);
		} else
		{
			color = vec4(192./255, 192./255, 192./255, 1);
		}
	}
}
